#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time
import threading

LOW, HIGH = 0, 1
PUD_OFF, PUD_HIGH, PUD_LOW = 1, 2, 3
INPUT, OUTPUT = 6, 7
CONSTANT_EDGE, RAISING_EDGE, FALLING_EDGE, EITHER_EDGE = 0, 8, 16, 24

NAMES = {
	PUD_OFF: 'PUD_OFF',
	PUD_HIGH: 'PUD_HIGH',
	PUD_LOW: 'PUD_LOW',
	INPUT: 'INPUT',
	OUTPUT: 'OUTPUT',
	CONSTANT_EDGE: 'CONSTANT_EDGE',
	RAISING_EDGE: 'RAISING_EDGE',
	FALLING_EDGE: 'FALLING_EDGE',
	EITHER_EDGE: 'EITHER_EDGE',
}

pin_state = [HIGH] * 30
pin_direction = [INPUT] * 30
pin_pull = [PUD_OFF] * 30
pin_callbacks = [None] * 30

running = threading.Event()
running.set()


# Mock Interface.

class pi(object):
	"""Minimal mock of the pigpio.pi class."""

	class callback_handle(object):
		"""Mock handle when registering callbacks."""

		def cancel(self):
			"""Mocked to match pigpio."""
			pass

	connected = True

	def set_mode(self, pin, mode):
		"""Set the direction/mode of pin."""
		pin_direction[pin] = mode

	def set_pull_up_down(self, pin, pull):
		"""Set pull up/down of pin. We just track how it is set, we don't actully pull the pins."""
		pin_pull[pin] = pull

	def read(self, pin):
		"""Read current value."""
		return _read(pin)

	def write(self, pin, value):
		"""Write to pin."""
		if pin_direction[pin] != OUTPUT:
			raise RuntimeError('Pin {d} is not in output mode.'.format(pin))
		_write(pin, value)

	def callback(self, pin, edge, callback):
		"""Register callbacks."""
		pin_callbacks[pin] = callback
		return pi.callback_handle()  # Mock handle

	def close(self):
		"""Close the object for shutdown."""
		running.clear()

	cancel = close


# Internals

def _trigger_callback(pin, edge):
	"""Fire the callback for the given pin/edge.

	TODO: Support the edge callback filtering of pigpio.
	"""
	func = pin_callbacks[pin]
	if func:
		func(pin, edge, 0)


def _write(pin, value):
	"""Write values to the pins. Update edges and callbacks."""
	edge = CONSTANT_EDGE

	if value > pin_state[pin]:
		edge |= RAISING_EDGE
	if value < pin_state[pin]:
		edge |= FALLING_EDGE
	pin_state[pin] = value

	if edge:
		# TODO: Check callback was looking for this type of edge.
		_trigger_callback(pin, value)


def _read(pin):
	"""Return the pin value."""
	return pin_state[pin]

# Simulation speed.
speed = 0.1


def _recieve_i2c():
	"""Mock up the pins recieving an I2C signal."""

	SCL = 3
	SDA = 2

	address = 0x14
	rw = 1  # read

	message = '[{0:08b} {1:08b} {2:08b} ]'.format((address << 1) | rw, 2, 63)
	print 'mock _recieve_i2c %s' % message

	clock_high = True
	_write(SCL, HIGH)
	_write(SDA, HIGH)
	time.sleep(speed)

	started = False
	going_to_stop = False
	going_to_start = False
	while running.is_set():
		if not message and not going_to_stop:
			break

		# Pulse the clock
		if clock_high:
			_write(SCL, HIGH)
		else:
			if started:
				_write(SCL, LOW)
		time.sleep(speed / 2.0)

		if _read(SCL):
			# Hold data
			if message and message[0] == '[':
				message = message[1:]
				going_to_start = True
			if going_to_start:
				# print 'start'
				# scl high then data falls
				_write(SDA, LOW)
				started = True
				going_to_start = False

			if going_to_stop:
				# scl high then data raises
				_write(SDA, HIGH)
				started = False
				going_to_stop = False

			# Hold data
			time.sleep(speed / 2.0)

		else:
			# Clock low, change data
			char = message[0]
			message = message[1:]
			going_to_stop = False
			# print 'char', char
			# Change data
			if char == '0':
				_write(SDA, LOW)

			elif char == '1':
				_write(SDA, HIGH)

			elif char == ' ':
				# should have ack, set high as the other end will pull low
				_write(SDA, HIGH)

			elif char == '[':
				_write(SDA, HIGH)
				going_to_start = True
			
			elif char == ']':
				print 'char ]'
				_write(SDA, LOW)
				going_to_stop = True

			time.sleep(speed / 2.0)
		
		clock_high = not clock_high

	print 'mock _recieve_i2c %s sent!' % message
