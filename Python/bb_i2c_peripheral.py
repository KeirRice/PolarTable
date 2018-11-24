#!/usr/bin/env python
# -*- coding: utf-8 -*-
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

More reference:
	https://rheingoldheavy.com/i2c-signals/
	https://robot-electronics.co.uk/i2c-tutorial

GPIO
	https://pimylifeup.com/raspberry-pi-gpio/

sudo apt-get update
sudo apt-get upgrade
sudo apt-get install rpi.gpio
sudo apt-get install python-rpi.gpio
sudo raspi-config
lsmod | grep i2c_
lsmod | grep spi_


https://www.raspberrypi.org/forums/viewtopic.php?t=8999
# dialout is a user group
echo "4" > /sys/class/gpio/export
sudo nano /etc/rc.local
chgrp -R dialout /sys/class/gpio
chmod -R g+rw /sys/class/gpio


https://www.raspberrypi.org/forums/viewtopic.php?p=161013&sid=963ba2183a4c7f5363cc2b911cac8838#p161013
sudo groupadd gpio
sudo usermod -aG gpio <myusername>
su <myusername>
sudo chgrp gpio /sys/class/gpio/export
sudo chgrp gpio /sys/class/gpio/unexport
sudo chmod 775 /sys/class/gpio/export
sudo chmod 775 /sys/class/gpio/unexport


https://www.raspberrypi.org/forums/viewtopic.php?t=118879
Solution is to edit:
/lib/udev/rules.d/60-python-pifacecommon.rules and
/lib/udev/rules.d/60-python3-pifacecommon.rules (if they both exist) to be:

KERNEL=="spidev*", GROUP="spi", MODE="0660"
SUBSYSTEM=="gpio*", PROGRAM="/bin/sh -c 'chown -R root:gpio /sys/class/gpio && chmod -R 770 /sys/class/gpio; chown -R root:gpio /sys/devices/virtual/gpio && chmod -R 770 /sys/devices/virtual/gpio; chown -R root:gpio /sys/devices/platform/soc/*.gpio/gpio && chmod -R 770 /sys/devices/platform/soc/*.gpio/gpio'"

"""

import Queue
import struct
import time
import threading
import pigpio_monitor_probes
from pigpio_monitor_probes import SignalProbe, DataProbe
import argparse


class PinBase(object):
	"""Wrapper to swap modes when we need to read and write."""

	def __init__(self, pin, pi):
		"""Init."""
		self.pi = pi
		self.pin = pin
		self.mode = None

		self.setup()

	def setup(self):
		"""Initial setup."""
		return NotImplemented

	def set_mode(self, mode):
		"""Update the pin mode if it needs to change.
		
		We keep a local cache of the value and assume it wont get changed
		externally.
		"""
		return NotImplemented

	def __eq__(self, other):
		"""Equality checks."""
		if isinstance(other, Pin):
			return self.pin == other.pin
		return self.pin == other

	def read(self):
		"""Read from the pin."""
		return NotImplemented

	def write(self, value):
		"""Write to the pin."""
		return NotImplemented

	def __int__(self):
		"""Int."""
		return self.pin

	def __radd__(self, other):
		return other + int(self)
	def __rsub__(self, other):
		return other - int(self)
	def __rmul__(self, other):
		return other * int(self)
	def __rmod__(self, other):
		return other % int(self)
	def __rdivmod__(self, other):
		return divmod(other, int(self))
	def __rpow__(self, other):
		return other ** int(self)
	def __rlshift__(self, other):
		return other << int(self)
	def __rrshift__(self, other):
		return other >> int(self)
	def __rand__(self, other):
		return other & int(self)
	def __rxor__(self, other):
		return other ^ int(self)
	def __ror__(self, other):
		return other | int(self)

class PiPin(PinBase):
	"""Wrapper to swap modes when we need to read and write."""

	def __init__(self, pin, pi):
		"""Init."""
		super(PiPin, self).__init__(pin, pi)

	def setup(self):
		"""Initial setup."""
		self.pi.set_pull_up_down(self.pin, pigpio.PUD_UP)

	def set_mode(self, mode):
		"""Update the pin mode if it needs to change.
		
		We keep a local cache of the value and assume it wont get changed
		externally.
		"""
		if self.mode != mode:
			self.pi.set_mode(self.pin, mode)
			self.mode = mode

	def read(self):
		"""Read from the pin."""
		self.set_mode(pigpio.INPUT)
		return self.pi.read(self.pin)

	def write(self, value):
		"""Write to the pin."""
		self.set_mode(pigpio.OUTPUT)
		return self.pi.write(self.pin, value)

class RpiPin(PinBase):
	"""Wrapper to swap modes when we need to read and write."""

	def __init__(self, pin, pi):
		"""Init."""
		super(RpiPin, self).__init__(pin, pi)

	def setup(self):
		"""Initial setup."""
		self.pi.setup(self.pin, self.pi.OUT, self.pi.PUD_OFF)

	def set_mode(self, mode):
		"""Update the pin mode if it needs to change.
		
		We keep a local cache of the value and assume it wont get changed
		externally.
		"""
		if self.mode != mode:
			self.pi.setup(self.pin, mode, PUD_OFF)
			self.mode = mode

	def read(self):
		"""Read from the pin."""
		# self.set_mode(self.pin, self.pi.IN)
		return self.pi.input(self.pin)

	def write(self, output):
		"""Write to the pin."""
		# self.set_mode(self.pin, self.pi.OUT)
		return self.pi.output(self.pin, output)

ENABLE_PIGPIO = True
ENABLE_RPI = False

if ENABLE_PIGPIO:

	# import pigpio_mock as pigpio
	import pigpio

	Pin = PiPin

if ENABLE_RPI:

	import RPi.GPIO as GPIO
	GPIO.setmode(GPIO.BCM)
	HIGH = GPIO.HIGH
	LOW = GPIO.LOW
	PUD_OFF = GPIO.PUD_OFF

	Pin = RpiPin

# Globals
NACK = 1
ACK = 0

SDA, SCL = 2, 3
PIN_NAMES = {
	SDA: 'SDA',
	SCL: 'SCL',
}


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
			# self.pin.write(LOW)
			return self

		def __exit__(self, *args):
			"""Release control of the clock."""
			# self.pin.write(HIGH)
			pass

	def __init__(self, SCL, SDA, address):
		"""Init."""
		if ENABLE_PIGPIO:
			pi = pigpio.pi()             # exit script if no connection
			if not pi.connected:
				raise RuntimeError('No Connection')

		if ENABLE_RPI:
			pi = GPIO
	
		SignalProbe(SDA, pi.read).start()
		SignalProbe(SCL, pi.read).start()
		
		# Pins and address
		self.SCL = Pin(SCL, pi)
		self.SDA = Pin(SDA, pi)
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
		self._write_packet = None

		# State
		self.transaction_active = False
		self.address_recieved = False
		self.rw = None
		self.error = False
		self.fall_oneshots = list()
		self.rise_oneshots = list()

		self.ack = False

		self.debug_scl = u' '
		self.debug_sda = u' '

		if ENABLE_PIGPIO:
			# Callbacks from the hardware side
			self.cb1 = pi.callback(int(self.SDA), pigpio.EITHER_EDGE, self.hardware_callback)
			self.cb2 = pi.callback(int(self.SCL), pigpio.EITHER_EDGE, self.hardware_callback)

		if ENABLE_RPI:
			def hardware_callback_sda():
				self.hardware_callback(self.SDA, self.SDA.read(), 0)

			def hardware_callback_scl():
				self.hardware_callback(self.SCL, self.SCL.read(), 0)

			GPIO.add_event_detect(int(self.SDA), GPIO.BOTH)
			GPIO.add_event_callback(int(self.SDA), hardware_callback_sda)
			GPIO.add_event_detect(int(self.SCL), GPIO.BOTH)
			GPIO.add_event_callback(int(self.SCL), hardware_callback_scl)

		self.bit_monitor = DataProbe('Bits', 100, False)
		self.bit_monitor.start()
		self.data_monitor = DataProbe('Data', 101)
		self.data_monitor.start()

	def post_bit_monitor_char(self, char):
		if self.bit_monitor:
			self.bit_monitor.data(char)

	def post_data_monitor_char(self, data, tag=''):
		if self.data_monitor:
			self.data_monitor.data(data, tag)

	def cancel(self):
		"""Clean up the callbacks."""
		if ENABLE_PIGPIO:
			self.cb1.cancel()
			self.cb2.cancel()

			pigpio.pi().close()

		if ENABLE_RPI:
			GPIO.remove_event_detect(self.SDA)
			GPIO.remove_event_detect(self.SCL)

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
		if pin == int(self.SDA):
			if level == pigpio.HIGH:
				self.sda_rise()
			else:
				self.sda_fall()
		
		elif pin == int(self.SCL):
			if level == pigpio.HIGH:
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
		if self.fall_oneshots:
			func = self.fall_oneshots.pop(0)
			func()
	
	def scl_rise(self):
		"""SCL line just rose."""
		if self.rise_oneshots:
			func = self.rise_oneshots.pop(0)
			func()

	def watch_for_start(self):
		"""Start is when the SDA line falls when the SCL is held high."""
		if self.SCL.read() == pigpio.HIGH:
			if self.transaction_active:
				# This is a repeated start!
				# TODO: Maybe we can release some bytes here?
				# self.release_bytes_so_far()
				pass
				print 'continue transaction'
				self.post_bit_monitor_char('s')
				self.post_data_monitor_char('', tag='Repeated Start')
			else:
				# New transaction.
				print 'new transaction'
				self.post_bit_monitor_char('s')
				self.post_data_monitor_char('', tag='Start')
				self.transaction_active = True

			# Start our data read so we can get the address.
			self.address_recieved = False
			self.rise_oneshots.append(self.read_data)

	def watch_for_end(self):
		"""End is when the SDA line raises when the SCL is held high."""
		if self.SCL.read() == pigpio.HIGH:
			# Clear out the transaction state
			self.transaction_active = False
			self.address_recieved = False
			print 'end transaction'
			self.post_bit_monitor_char('e')
			self.post_data_monitor_char('', tag='End')

			self.release_bytes_so_far()

	# Read
	def read_data(self):
		"""Read data on the clock high pulses."""
		# If we are not in a tansaction the data is for someone else.
		# Don't read data generated when we are sending an ack
		if not self.transaction_active or self.ack:
			return
		
		with Peripheral.Stretch(self.SCL):
			bit = self.SDA.read()
			print 'bit', bit

			self.post_bit_monitor_char(str(bit))
			self._read_buffer = (self._read_buffer << 1) | bit
			self._read_counter += 1
			
			if self._read_counter == 8:
				# A whole byte has been read, check for an ack from the controller next clock tick.
				self._read_counter = 0
				packet = self._read_buffer
				self._read_buffer = 0
				self.process_packet(packet)
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
				self._write_packet = self._write_buffer = self.write_buffer.get_nowait()

			# Writing is MSB first
			self.SDA.write(1 if self._write_buffer & 0b10000000 else 0)
			self._write_buffer = self._write_buffer << 1
			self._write_counter += 1
			
			if self._write_counter == 8:
				# The byte has been written, check for an ack from the controller next clock tick.
				self.post_data_monitor_char(u'{:02x} '.format(self._write_packet), 'Write')
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
		self.ack = True
		print 'write_ack'
		self.SDA.write(pigpio.LOW)
		self.post_bit_monitor_char('a')
		self.post_data_monitor_char('', tag='Send Ack')

		def clear_ack():
			self.ack = False
			print 'clear_ack'
			self.SDA.write(pigpio.HIGH)
			self.rise_oneshots.append(self.read_data)

		self.fall_oneshots.append(clear_ack)

	def write_nack(self):
		"""Send a HIGH bit for the next SCL pulse.

		We probabily don't even need to do this much as doing nothing will
		also get interupted on the controll as a nack.
		"""
		print 'nack'
		self.SDA.write(pigpio.HIGH)

	# Process
	def process_packet(self, packet):
		"""Action the data we just got."""
		print 'process_packet'
		if self.address_recieved:
			# We have new data!
			self.fall_oneshots.append(self.write_ack)
			self.read_buffer.put_nowait(packet)
			self._bytes_recieved += 1

			self.post_data_monitor_char(packet, 'Read')

		else:
			# Grab the address
			# First frame after a start is always the address and Read/Write flag.
			if (self._read_buffer >> 2) == 0b11110:
				# We are not using 10bit addresses so we can safely ignore this transaction.
				return
			address = packet >> 1
			print 'address', address, bin(address)
			self.post_data_monitor_char(address, tag='Address')
			
			if address != self.our_address:
				self.transaction_active = False
				# Ignore any transactions not for us.
				return
			self.fall_oneshots.append(self.write_ack)
			self.rw = packet & 1

			# Setup the callback chain for the transaction.
			self.address_recieved = True
			if self.rw == 0:
				if self.onSlaveTransmit:
					self.onSlaveTransmit()
				self.post_data_monitor_char(0, tag='Write Mode')
				self.fall_oneshots.append(self.write_data)

			elif self.rw == 1:
				self.post_data_monitor_char(1, tag='Read Mode')
				self.rise_oneshots.append(self.read_data)

			else:
				raise RuntimeError('Whaaaa')

	# Python side interface
	def read(self, size=-1):
		"""Read from the read buffer and return a byte encoded string."""
		bytes_list = list()
		if size == -1:
			size = self.read_buffer.qsize()
		try:
			for i in range(size):
				bytes_list.append(self.read_buffer.get_nowait())  # += struct.pack('>I', self.read_buffer.get_nowait())
		except Queue.Empty:
			pass
		return bytes_list

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

	def __init__(self, name, size=1):
		"""Init."""
		self.name = name
		self.index = Register.REGISTER_COUNT
		Register.REGISTER_COUNT += 1
		Register.REGISTERS.append(self)

	def __int__(self):
		"""For indexing."""
		return self.index

	@classmethod
	def max(cls):
		"""Highest register offset."""
		return cls.REGISTER_COUNT


class CompoundRegister(object):
	"""Base for working with values that span multiple registers."""

	def __init__(self, name, *args):
		"""Init."""
		self.name = name
		self.registers = args

	def pack(self, value):
		"""Write a struct pack to get your value into bytes."""
		return NotImplemented

	def unpack(self, value):
		"""Write a struct unpack to get your bytes into value."""
		return NotImplemented


class IntRegister(CompoundRegister):
	"""Class for auto numbering registers."""

	def pack(self, value):
		"""Short int to bytes."""
		return struct.pack('>h', value)

	def unpack(self, value):
		"""Byte to short int."""
		return struct.unpack('>h', value)[0]
		

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

REG_MOTOR_THETA = IntRegister('REG_MOTOR_THETA', REG_MOTOR_T1, REG_MOTOR_T2)
REG_MOTOR_RADIUS = IntRegister('REG_MOTOR_RADIUS', REG_MOTOR_R1, REG_MOTOR_R2)


class Registers(object):
	"""Storage for registers accessiable from python and I2C."""

	def __init__(self, wire):
		"""Init."""
		self.wire = wire
		wire.onReceive(self.i2cReceiveEvent)
		wire.onRequest(self.i2cRequestEvent)

		self.reg_position = 0
		self.register_data = bytearray(Register.max())

	# I2C interface
	def i2cReceiveEvent(self, how_many):
		"""We have new data to handle."""
		print 'i2cReceiveEvent', how_many
		if how_many < 1 or not self.wire.available():
			return

		reg_position = int(self.wire.read(1)[0])  # struct.unpack('>I', self.wire.read())[0]
		if 0 > reg_position > len(self.register_data):
			raise RuntimeError("Fail we don't have a register at {0}".format(reg_position))
		self.reg_position = reg_position

		how_many -= 1
		if not how_many:
			return

		registers_changed = list()
		while how_many and self.wire.available():
			# Set the value
			self.register_data[self.reg_position] = self.wire.read(1)[0]
			registers_changed.append(self.reg_position)
			self.increment_register_position()
			how_many -= 1

		self.process_callbacks(registers_changed)

	def i2cRequestEvent(self):
		"""We need to send some data."""
		print 'i2cRequestEvent', self.reg_position
		self.wire.write(self.registers[self.reg_position])
		self.increment_register_position()

	# Python side interface
	def read(self, register):
		"""Read a register."""
		if isinstance(register, Register):
			return self.register_data[register]

		elif isinstance(register, CompoundRegister):
			values = b''
			for reg in register.registers:
				values += self.register_data[reg]
			return register.unpack(values)

		raise RuntimeError('Not supported')
	
	def write(self, register, value):
		"""Write to a register."""
		if isinstance(register, CompoundRegister):
			for reg, val in zip(register.registers, register.pack(value)):
				self.write(reg, val)
			return

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
			print 'register', register, self.register_data[register]


def main():
	"""Run."""

	parser = argparse.ArgumentParser()
	
	parser.add_argument(
		"-s",
		"--server-ip",
		help="IP address of the server running the PGIO Monitor")
	
	parser.add_argument(
		"-a",
		"--i2c-address",
		help="Our device address in hex.")

	parser.add_argument(
		"-d",
		"--sda",
		help="SDA Pin.")
	parser.add_argument(
		"-c",
		"--scl",
		help="SCL Pin.")

	args = parser.parse_args()
	
	global SERVER
	if args.server_ip:
		SERVER = (args.server_ip, pigpio_monitor_probes.UDP_PORT)
		pigpio_monitor_probes.set_server(SERVER)

	global ADDRESS
	if args.i2c_address:
		ADDRESS = int(args.i2c_address, 16)
	else:
		ADDRESS = 0x14

	global SDA, SCL
	if args.sda:
		SDA = int(args.sda)
		PIN_NAMES[SDA] = 'SDA'
	if args.scl:
		SCL = int(args.scl)
		PIN_NAMES[SCL] = 'SCL'

	print 'listening: Address = ', ADDRESS, 'PINS', SDA, SCL
	wire = Peripheral(SCL, SDA, ADDRESS)

	try:
		reg = Registers(wire)
		# threading.Thread(target=pigpio._recieve_i2c).start()

		while True:
			time.sleep(1)

		# reg.write(REG_MOTOR_THETA, 654654)
		# reg.write(REG_MOTOR_RADIUS, 654654)

	except KeyboardInterrupt:
		print 'KeyboardInterrupt'
		wire.cancel()
		

if __name__ == '__main__':
	main()
