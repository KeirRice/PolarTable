#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time
import threading
import socket
import curses

import locale
locale.setlocale(locale.LC_ALL, '')
code = locale.getpreferredencoding()


def encoder(string):
	"""Match the system encoding."""
	return string.encode(code)


UDP_IP = "127.0.0.1"
UDP_PORT = 6000

# Simulation speed.
SIMULATION_SPEED = 0.1
SAMPLE_SPEED = SIMULATION_SPEED / 10.0

START_TIME = time.time()


def millis():
	"""Milli seconds since script start."""
	return int((time.time() - START_TIME) * 1000)


# Client Side

class Probe(threading.Thread):
	"""Monitor the levels of a given PIN."""

	def __init__(self, port_offset):
		"""Init."""
		super(SignalProbe, self).__init__()
		
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.address = (UDP_IP, UDP_PORT + port_offset)

		self.daemon = True

	def run(self):
		"""Main loop catches data and broadcasts it."""
		return NotImplemented

	def broadcast(self, char):
		"""Send char out over UDP, hopfully to a listening monitor."""
		encoded = char.encode('utf-8')
		self.sock.sendto(encoded, self.address)


class SignalProbe(Probe):
	"""Monitor the levels of a given PIN."""

	def __init__(self, pin, read_func):
		"""Init."""
		port_offset = pin
		super(SignalProbe, self).__init__(port_offset)
		self.pin = pin
		self.read_func = read_func
		self.pin_buffer = u''

	def run(self):
		"""Main loop catches data and broadcasts it."""
		symbol_map = {
			u'/': u'‾',
			u'‾': u'‾',
			u'_': u'_',
			u'\\': u'_',
			u' ': u' ',
			0: u'_',
			1: u'‾',
		}
		last_state = None
		while True:
			state = self.read_func(self.pin)
			char = symbol_map[state]

			if last_state is not None and last_state != state:
				if state == 1:
					char = u'/'
				elif state == 0:
					char = u'\\'

			self.pin_buffer += char
			self.broadcast(char)

			last_state = state
			time.sleep(SAMPLE_SPEED)


class DataProbe(threading.Thread):
	"""Instrument code with this class to send data back to the monitor."""

	def __init__(self, name, port_offset, compact=True):
		"""Init."""
		super(DataProbe, self).__init__(port_offset)
		self.daemon = True
		
		self.name = name
		
		# If compact is False we insert spaces into the stream so it keeps
		# up with the sample rate.
		self.compact = compact
		self.char = None

	def data(self, char):
		"""Call this with data you want to monitor."""
		self.char = char

	def run(self):
		"""Main loop catches data and broadcasts it."""
		while True:
			if self.char is None:
				if self.compact is False:
					self.broadcast(' ')
			else:
				self.broadcast(self.char)
				self.char = None
			time.sleep(SAMPLE_SPEED)


# Server Side


class Signal(threading.Thread):
	"""Base class for different types of signal collectors."""

	__signals = list()

	def __init__(self, port, name):
		"""Init."""
		super(Signal, self).__init__()
		self.daemon = True

		self.name = name
		
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDP
		self.sock.bind((UDP_IP, port))

		self.data = list()
		self.millis = list()
		self.line = 0
		self.new_data = False

		Signal.__signals.append(self)

	@classmethod
	def listen(cls):
		"""Start up all the signals."""
		for s in cls.__signals:
			s.start()

	def line_hint(self):
		"""The line number to display on."""
		# TODO: Decouple this...
		return self.line

	def run(self):
		"""Read incomming data and cach it for display."""
		while True:
			data, addr = self.sock.recvfrom(1024)  # buffer size is 1024 bytes
			incomming = data.decode('utf-8')
			self.data.append(incomming)
			self.millis.append(millis())
			self.new_data = True

	def present(self, width):
		"""Return the most recent cached data upto the width."""
		return NotImplemented

"""
********************************************************************************
I2C Watcher
********************************************************************************

SDA(2) /‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\____________________________________________________________
SCL(3) _____/‾‾‾‾‾‾‾‾\________/‾‾‾‾‾‾‾‾‾\________/‾‾‾‾‾‾‾‾\________/‾‾‾‾‾‾‾‾‾\________/
BIT                           0                  0                  0                 0

********************************************************************************
Time	 	Tag				Binary				Hex				Decimal			Ascii
	
30			Address			0b010100			0x14			20			



Commands: (P)ause, (←) Scrub Left, (→) Scrub Right, 





--


return chr(c) if curses.ascii.isalnum(c) else ''
"""
class DataSignal(Signal):
	"""Data signals. Anything text based."""

	def __init__(self, name, port_offset, line):
		"""Init."""
		super(DataSignal, self).__init__(UDP_PORT + port_offset, name)
		self.line = line

	def present_timeline(self, width):
		"""Return the most recent cached data upto the width."""
		self.new_data = False

		return u'{label:<10} {data}'.format(
			label=self.name,
			data=self.data if len(self.data) < width else self.data[-width:],
		)

	def present_table(self):
		"""Return the most recent cached data upto the width."""

		for data, timestamp in zip(self.)
		timestamp = 

		return u'{label:<10} {data}'.format(
			label=self.name,
			data=self.data if len(self.data) < width else self.data[-width:],
		)


class PinSignal(Signal):
	"""Pin signals. Shows HIGH or LOW as well as transistions."""

	def __init__(self, name, pin):
		"""Init."""
		super(PinSignal, self).__init__(UDP_PORT + pin, name)
		self.line = self.pin = pin

	def present_timeline(self, width):
		"""Return the most recent cached data upto the width."""
		self.new_data = False

		label = u'{name}({pin})'.format(name=self.name, pin=self.pin)
		
		data = u''.join(self.data)
		data = data if len(data) < width else data[-width:]

		return u'{label:<10} {data}'.format(label=label, data=data)


def main():
	ur"""Setup a window to work in.

	********************************************************************************
	I2C Watcher
	********************************************************************************



	SDA(2) /‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\____________________________________________________________
	SCL(3) _____/‾‾‾‾‾‾‾‾\________/‾‾‾‾‾‾‾‾‾\________/‾‾‾‾‾‾‾‾\________/‾‾‾‾‾‾‾‾‾\________/
	BIT                           0                  0                  0                 0

	DATA 14

	"""
	stdscr = curses.initscr()
	curses.noecho()
	curses.cbreak()
	stdscr.keypad(1)

	try:
		# Window size.
		begin_x = 20
		begin_y = 7
		height = 30
		width = 80
		win = curses.newwin(height, width, begin_y, begin_x)
		max_height, max_width = win.getmaxyx()
		
		# Debug line.
		# stdscr.addstr(max_height, 0, 'max_height {}, max_width {}'.format(max_height, max_width))

		signals = [
			PinSignal('SDA', 2),  # SDA signal pin
			PinSignal('SCL', 3),  # SCL signal pin
			DataSignal('BIT   ', 100, 4),  # The bits as they get read
			DataSignal('DATA', 101, 6),  # The data in hex
		]

		# Start listening
		Signal.listen()

		# Header
		stdscr.addstr(0, 0, '*' * max_width)
		stdscr.addstr(1, 0, 'I2C Watcher')
		stdscr.addstr(2, 0, '*' * max_width)
		stdscr.refresh()

		# Update the data streams
		offset = 4
		new_data = True
		while True:
			
			for s in signals:
				if s.new_data:
					new_data = True
					break

			if new_data:
				for s in signals:
					text = encoder(s.present(max_width))
					stdscr.addstr(offset + s.line_hint(), 0, text)
					# stdscr.redrawln(offset + s.pin)
				stdscr.refresh()
				new_data = False

	finally:
		stdscr.keypad(0)
		curses.nocbreak()
		curses.echo()
		curses.endwin()

if __name__ == '__main__':
	main()
