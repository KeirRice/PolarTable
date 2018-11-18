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

class callback_handle(object):

	def cancel(self):
		pass

class pi(object):

	connected = True

	def set_mode(self, pin, mode):
		pin_direction[pin] = mode

	def set_pull_up_down(self, pin, pull):
		pin_pull[pin] = pull

	def read(self, pin):
		return _read(pin)

	def write(self, pin, value):
		if pin_direction[pin] != OUTPUT:
			raise RuntimeError('Pin {d} is not in output mode.'.format(pin))
		_write(pin, value)

	def callback(self, pin, edge, callback):
		# callback(pin, level, tick)
		pin_callbacks[pin] = callback
		return callback_handle()  # should be a handle

	def close(self):
		running.clear()
	cancel = close


def _trigger_callback(pin, edge):
	func = pin_callbacks[pin]
	if func:
		func(pin, edge, 0)

def _write(pin, value):
	# print '_write(', pin, value, ')'
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
	return pin_state[pin]

speed = 0.1

def _recieve_i2c():

	SCL = 3
	SDA = 2

	address = 0x14
	rw = 1  # read

	message = '[{0:08b} {1:08b}]'.format((address << 1) | rw, 2)
	print 'mock _recieve_i2c %s' % message

	clock_high = True
	_write(SCL, HIGH)
	_write(SDA, HIGH)
	time.sleep(speed)

	while running.is_set():

		if not message:
			break
		
		if message[0] == '[':
			# print 'start'
			# scl high then data falls
			_write(SCL, HIGH)
			time.sleep(speed / 2.0)
			_write(SDA, LOW)
			time.sleep(speed / 2.0)
			message = message[1:]

			continue

		elif message[0] == ']':
			# print 'end'
			# scl high then data raises
			_write(SCL, HIGH)
			time.sleep(speed / 2.0)
			_write(SDA, HIGH)
			time.sleep(speed / 2.0)
			message = message[1:]

			continue

		# Pulse the clock
		if clock_high:
			_write(SCL, HIGH)
		else:
			_write(SCL, LOW)
		time.sleep(speed / 2.0)

		if clock_high:
			# Hold data
			time.sleep(speed / 2.0)

		else:
			char = message[0]
			message = message[1:]
			# print 'char', char
			# Change data
			if char == '0':
				_write(SDA, LOW)

			elif char == '1':
				_write(SDA, HIGH)

			elif char == ' ':
				# should have ack, set high as the other end will pull low
				_write(SDA, HIGH)

			time.sleep(speed / 2.0)
		
		clock_high = not clock_high

	print 'mock _recieve_i2c %s sent!' % message


class MonitorPin(threading.Thread):

	def __init__(self, pin):
		super(MonitorPin, self).__init__()
		self.pin = pin
		self.pin_buffer = u''

		self.end = threading.Event()

	def run(self):
		pin = self.pin
		sample_speed = speed / 10.0
		symbol_map = {
			u'/': u'‾',
			u'‾': u'‾',
			u'_': u'_',
			u'\\': u'_',
			u' ': u' ',
			LOW: u'_',
			HIGH: u'‾',
		}
		last_state = None
		while not self.end.is_set():
			state = _read(pin)
			if last_state is None:
				self.pin_buffer = symbol_map[state]
			else:
				if last_state != state:
					if state == HIGH:
						self.pin_buffer += u'/'
					elif state == LOW:
						self.pin_buffer += u'\\'
					else:
						raise RuntimeError('What')

				else:
					self.pin_buffer += symbol_map[state]

			last_state = state
			time.sleep(sample_speed)

	def stop(self):
		self.end.set()

	def __unicode__(self):
		return self.pin_buffer


def delay():
	target = time.time() + 0.0001
	while target > time.time():
		pass
