#!/usr/bin/env python
"""Bitbashed IC2 communication as a peripheral (Slave mode).

# Transaction
Start
	# Address
	7 or 10 address bits
		SDA	A6	A5	A4	A3	A2	A1	A0	R/W
		SDL	1	2	3	4	5	6	7	8
	1 read/write bit
	1 ACK/NACK
	
	# Packet
	8 Data frame
	1 ACK/NACK

	# Optional aditional packets
	8 Data frame
	1 ACK/NACK

Start  # Optional repeat start without Stop
	# Address
	7 or 10 address bits
		SDA	A6	A5	A4	A3	A2	A1	A0	R/W
		SDL	1	2	3	4	5	6	7	8
	1 read/write bit
	1 ACK/NACK
	
	# Packet
	8 Data frame
	1 ACK/NACK

	# Optional aditional packets
	8 Data frame
	1 ACK/NACK

Stop

"""


import pigpio
import Queue


# Globals

HIGH = NACK = 1
LOW = ACK = 0


class Peripheral(object):
	"""Peripheral is a class for bit-bashing an I2C "slave" connection."""

	class Stretch(object):
		"""Hold the clock low while we are in the code block.

		# TODO: Can we add a watchdog to time out the stretch if it takes too long?
		"""

		def __init__(self, pin):
			"""Init."""
			self.pin = pin

		def __enter__(self):
			"""Pull low for clock streching."""
			self.pin.write(LOW)
			return self

		def __exit__(self, *args):
			"""Release control of the clock."""
			self.pin.write(HIGH)

	class Pin(object):
		"""Wrapper to swap modes when we need to read and write."""

		def __init__(self, pin, pi):
			"""Init."""
			self.pi = pi
			self.pin = pin
			self.mode = self.pi.get_mode(pin)
			self.pi.set_pull_up_down(pin, pigpio.PUD_OFF)

		def set_mode(self, mode):
			"""Update the pin mode if it needs to change.
			
			We keep a local cache of the value and assume it wont get changed
			externally.
			"""
			if self.mode != mode:
				self.pi.set_mode(mode)
				self.mode = mode

		def __eq__(self, other):
			"""Equality checks."""
			if isinstance(other, Peripheral.Pin):
				return self.pin == other.pin
			return self.pin == other

		def read(self):
			"""Read from the pin."""
			self.set_mode(self.pin, pigpio.INPUT)
			return self.pi.read(self.pin)

		def write(self):
			"""Write to the pin."""
			self.set_mode(self.pin, pigpio.OUTPUT)
			return self.pi.write(self.pin)

	def __init__(self, SCL, SDA, address):
		"""Init."""
		pi = pigpio.pi()             # exit script if no connection
		if not pi.connected:
			raise RuntimeError('No Connection')
		
		# Pins and address
		self.SCL = Peripheral.Pin(SCL, pi)
		self.SDA = Peripheral.Pin(SDA, pi)
		self.our_address = address

		# Callbacks for the python side
		self.onSlaveReceive = None
		self.onSlaveTransmit = None

		# Buffers
		self.read_buffer = Queue.Queue()
		self._read_counter = 0
		self._read_buffer = 0
		self._bytes_recieved = 0

		self.write_buffer = Queue.Queue()
		self._write_counter = 0
		self._write_buffer = 0

		# State
		self.transaction_active = False
		self.address_recieved = False
		self.rw = None
		self.error = False
		self.fall_oneshots = list()
		self.rise_oneshots = list()

		# Callbacks from the hardware side
		self.cb1 = self.pi.callback(self.SDA, pigpio.EITHER_EDGE, self.hardware_callback)
		self.cb2 = self.pi.callback(self.SCL, pigpio.EITHER_EDGE, self.hardware_callback)

	def cancel(self):
		"""Clean up the callbacks."""
		self.cb1.cancel()
		self.cb2.cancel()

	def reset_state(self):
		"""Clear out any state for a next transaction."""
		# Clear byte buffers
		self._read_counter = 0
		self._read_buffer = 0
		self._write_counter = 0
		self._write_buffer = 0

		# Clear state
		self.transaction_active = False
		self.address_recieved = False
		self.error = False
		self.fall_oneshots = list()
		self.rise_oneshots = list()

	# Hardware side callbacks.
	def hardware_callback(self, pin, level, tick):
		"""Callback on pin change from pigpio."""
		if pin == self.SDA:
			if level:
				self.sda_rise()
			else:
				self.sda_fall()
		elif pin == self.SCL:
			if level:
				self.scl_rise()
			else:
				self.scl_fall()

	def sda_fall(self):
		"""SDA line just fell."""
		self.watch_for_start()
	
	def sda_rise(self):
		"""SDA line just rose."""
		self.watch_for_end()
	
	def scl_fall(self):
		"""SCL line just fell."""
		while len(self.fall_oneshots):
			self.fall_oneshots.pop()()
	
	def scl_rise(self):
		"""SCL line just rose."""
		while len(self.rise_oneshots):
			self.rise_oneshots.pop()()

	def watch_for_start(self):
		"""Start is when the SDA line falls when the SCL is held high."""
		if self.SCL.read() == HIGH:
			if self.transaction_active:
				# This is a repeated start!
				# TODO: Maybe we can release some bytes here?
				# self.release_bytes_so_far()
				pass
			else:
				# New transaction.
				self.transaction_active = True

			# Start our data read so we can get the address.
			self.address_recieved = False
			self.rise_oneshots.append(self.read_data)

	def watch_for_end(self):
		"""End is when the SDA line raises when the SCL is held high."""
		if self.SCL.read() == HIGH:
			# Clear out the transaction state
			self.transaction_active = False
			self.address_recieved = False

			self.release_bytes_so_far()

	# Read
	def read_data(self):
		"""Read data on the clock high pulses."""
		# If we are not in a tansaction the data is for someone else.
		if not self.transaction_active:
			return
		
		with Peripheral.Stretch(self.SCL):
			self._read_buffer = (self._read_buffer << 1) | self.SDA.read()
			self._read_counter += 1

			if self._read_counter == 8:
				# A whole byte has been read, check for an ack from the controller next clock tick.
				self._read_counter = 0
				self.fall_oneshots.append(self.write_ack)
				self.process_packet()
			else:
				# Haven't finished the byte yet, keep going.
				self.rise_oneshots.append(self.read_data)

	def read_nack(self):
		"""Set an error flag if we get a nack."""
		# TODO: How to handle errors
		self.error = self.SDA.read() == NACK
	
	def read_ack(self):
		"""Clear error flag if we get a ack."""
		self.error = self.SDA.read() == ACK

	# Write
	def write_data(self):
		"""Write data for the controller."""
		if not self.transaction_active:
			return

		with Peripheral.Stretch(self.SCL):
			if self._write_buffer is None:
				self._write_buffer = self.write_buffer.get_nowait()

			# Writing is MSB first
			self.SDA.write(self._write_buffer & 0b10000000)
			self._write_buffer = self._write_buffer << 1
			self._write_counter += 1
			
			if self._write_counter == 8:
				# The byte has been written, check for an ack from the controller next clock tick.
				self._write_counter = 0
				self._write_buffer = None
				self.rise_oneshots.append(self.read_nack)
			else:
				# Haven't finished the byte yet, keep going.
				self.fall_oneshots.append(self.write_data)

	def write_ack(self):
		"""Send a low bit for the next SCL pulse.

		1) Write data LOW
		2) Hold LOW while the clock pulse is HIGH
		3) When clock falls, clear out our ack by writing high
		4) Switch back to read mode ready for the next clock pulse
		"""
		self.SDA.write(LOW)

		def clear_ack():
			self.SDA.write(HIGH)
			self.rise_oneshots.append(self.read_data)
		self.fall_oneshots.append(clear_ack)

	def write_nack(self):
		"""Send a HIGH bit for the next SCL pulse.

		We probabily don't even need to do this much as doing nothing will
		also get interupted on the controll as a nack.
		"""
		self.SDA.write(HIGH)

	# Process
	def process_packet(self):
		"""Action the data we just got."""
		if self.address_recieved:
			# We have new data!
			self.read_buffer.put_nowait(self._read_buffer)
			self._bytes_recieved += 1

		else:
			# Grab the address
			# First frame after a start is always the address and Read/Write flag.
			if (self._read_buffer >> 2) == 0b11110:
				# We are not using 10bit addresses so we can safely ignore this transaction.
				return
			address = self._read_buffer >> 1
			if address != self.our_address:
				self.transaction_active = False
				# Ignore any transactions not for us.
				return
			self.rw = self._read_buffer & 1

			# Setup the callback chain for the transaction.
			self.address_recieved = True
			if self.rw == Peripheral.CONTROLLER_REQUESTING:
				if self.onSlaveTransmit:
					self.onSlaveTransmit()
				self.fall_oneshots.append(self.write_data)
			elif self.rw == Peripheral.CONTROLLER_REQUESTING:
				self.rise_oneshots.append(self.read_data)

	# Python side interface
	def read(self, size=-1):
		"""Read from the read buffer and return a byte encoded string."""
		byte_string = b''
		if size == -1:
			size = self.read_buffer.qsize()
		try:
			for i in range(size):
				byte_string += chr(self.read_buffer.get_nowait())
		except Queue.Empty:
			pass
		return byte_string

	def write(self, value):
		"""Write bytes in to the buffer for sending."""
		if isinstance(value, int):
			self.write_buffer.put_nowait(value)
		elif isinstance(value, basestring):
			value = bytes(value, 'utf8')
		
		if isinstance(value, bytes):
			for val in value:
				self.write_buffer.put_nowait(val)
		
	def available(self):
		"""Number of bytes in the read buffer."""
		return self.read_buffer.qsize()

	def release_bytes_so_far(self):
		"""Fire the callback and let the app handle any bytes so far.

		TODO: Test if we have enough time for the app to read the values and respond
		to the next repeated start immediately.
		"""
		if self._bytes_recieved:
			if self.onSlaveReceive:
				self.onSlaveReceive(self._bytes_recieved)
			self._bytes_recieved = 0

	# Register python side callbacks
	def onReceive(self, func):
		"""We have data and can read it out."""
		self.onSlaveReceive = func

	def onRequest(self, func):
		"""Time to call write and fill the buffer."""
		self.onSlaveTransmit = func


class Register(object):
	"""Class for auto numbering registers."""

	REGISTERS = list()
	REGISTER_COUNT = 0

	def __init__(self, name):
		"""Init."""
		self.name = name
		self.index = Register.REGISTER_COUNT
		Register.REGISTER_COUNT += 1
		Register.REGISTERS.append(self)

	@classmethod
	def max(cls):
		"""Highest register offset."""
		return cls.REGISTER_COUNT


# Create the list of registers we are exposing.
REG_STATUS = Register('REG_STATUS')
REG_SYS_SLEEP = Register('REG_SYS_SLEEP')  # System State (Sleep/Wake)
REG_LIGHTING_ON = Register('REG_LIGHTING_ON')  # Lighting On/Off
REG_LIGHTING_R = Register('REG_LIGHTING_R')	 # Lighting Red
REG_LIGHTING_G = Register('REG_LIGHTING_G')	 # Lighting Green
REG_LIGHTING_B = Register('REG_LIGHTING_B')	 # Lighting Blue
REG_LIGHTING_TIME = Register('REG_LIGHTING_TIME')  # Lighting Blend Time
REG_MOTOR_T1 = Register('REG_MOTOR_T1')	 # Motor Theta Steps MSB
REG_MOTOR_T2 = Register('REG_MOTOR_T2')	 # Motor Theta Steps LSB
REG_MOTOR_R1 = Register('REG_MOTOR_R1')	 # Motor Radius Steps MSB
REG_MOTOR_R2 = Register('REG_MOTOR_R2')	 # Motor Radius Steps LSB

REG_INTERUPTS_0 = Register('REG_INTERUPTS_0')  # Interupts Bank0
REG_INTERUPTS_1 = Register('REG_INTERUPTS_1')  # Interupts Bank1


class Registers(object):
	"""Storage for registers accessiable from python and I2C."""

	def __init__(self, wire):
		"""Init."""
		self.wire = wire
		self.onReceive(self.i2cReceiveEvent)
		self.onRequest(self.i2cRequestEvent)

		self.reg_position = 0
		self.register_data = bytearray(Register.max())

	# I2C interface
	def i2cReceiveEvent(self, how_many):
		"""We have new data to handle."""
		if how_many < 1 or not self.wire.available():
			return

		self.reg_position = self.wire.read()
		if self.reg_position not in self.register_data:
			raise RuntimeError("Fail we don't have a register at that offset.")

		how_many -= 1
		if not how_many:
			return

		registers_changed = list()
		while how_many and self.wire.available():
			# Set the value
			self.registers[self.reg_position] = self.wire.read()
			registers_changed.append(self.reg_position)
			self.increment_register_position()
			how_many -= 1

		self.process_callbacks(registers_changed)

	def i2cRequestEvent(self):
		"""We need to send some data."""
		self.wire.write(self.registers[self.reg_position])
		self.increment_register_position()

	# Python side interface
	def read(self, register):
		"""Read a register."""
		return self.register_data[register]
	
	def write(self, register, value):
		"""Write to a register."""
		self.register_data[register] = value

		# Update our interupt flags
		bank, bit = divmod(register, 8)
		self.register_data[REG_INTERUPTS_0 + bank] |= 1 << bit

	# Helpers
	def increment_register_position(self):
		"""Increment register position."""
		self.reg_position += 1
		if self.reg_position >= Register.max():
			self.reg_position = 0

	def process_callbacks(self, registers_changed):
		"""Callback for the python side to know when data was updated."""
		for register in registers_changed:
			# TODO: Call out to system that needs to update.
			pass


def main():
	SDA, SCL = 4, 5
	ADDRESS = '\x14'
	wire = Peripheral(SCL, SDA, ADDRESS)
	try:
		Registers(wire)
		while True:
			pass
	finally:
		wire.cancel()
