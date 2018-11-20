#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time
import threading
import socket
import curses
import json
import Queue
from terminalsize import get_terminal_size

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
		super(Probe, self).__init__()
		
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.address = (UDP_IP, UDP_PORT + port_offset)

		self.daemon = True

	def run(self):
		"""Main loop catches data and broadcasts it."""
		return NotImplemented

	def broadcast(self, char, tag=''):
		"""Send char out over UDP, hopfully to a listening monitor."""
		outgoing = json.dumps({
			'data': char,
			'tag': tag.encode('utf-8')
		})
		self.sock.sendto(outgoing, self.address)


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


class DataProbe(Probe):
	"""Instrument code with this class to send data back to the monitor."""

	def __init__(self, name, port_offset, compact=True):
		"""Init."""
		super(DataProbe, self).__init__(port_offset)
		self.daemon = True
		
		self.name = name
		
		# If compact is False we insert spaces into the stream so it keeps
		# up with the sample rate.
		self.compact = compact
		self.char = Queue.Queue()

	def data(self, char, tag=''):
		"""Call this with data you want to monitor."""
		self.char.put((char, tag))

	def run(self):
		"""Main loop catches data and broadcasts it."""
		while True:
			if self.char is None:
				if self.compact is False:
					self.broadcast(' ')
			else:
				try:
					while True:
						char, tag = self.char.get_nowait()
						print 'broadcast', char, tag
						self.broadcast(char, tag)
				except Queue.Empty:
					pass
			time.sleep(SAMPLE_SPEED)


class ContextProbe(Probe):
	"""Instrument code with this class to send data back to the monitor."""

	pass


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
			incomming = json.loads(data.decode('utf-8'))
			incomming['timestamp'] = millis()
			self.data.append(incomming)
			self.new_data = True

	def present_timeline(self, skip=0, limit=-1):
		"""Return the most recent cached data upto the width."""
		return NotImplemented

	def present_table(self, skip=0, limit=-1):
		"""Return the most recent cached data upto the width."""
		return NotImplemented

	def pause(self, pause):
		"""Pause data capture?"""
		pass


class DataSignal(Signal):
	"""Data signals. Anything text based."""

	def __init__(self, name, port_offset, line):
		"""Init."""
		super(DataSignal, self).__init__(UDP_PORT + port_offset, name)
		self.line = line

	def present_timeline(self, skip=0, limit=-1):
		"""Return the most recent cached data upto the width."""
		skip = min(skip, len(self.data))

		if limit < 0:
			limit = len(self.data)
		limit = min(limit, len(self.data) - skip)
		
		data = list(reversed(self.data))[skip:skip + limit]
		data = [unicode(d['data']) for d in reversed(data)]
		
		return u'{label:<10} {data}'.format(
			label=self.name,
			data=u''.join(data)
		)

	def present_table(self, skip=0, limit=-1, context=None):
		"""Return the most recent cached data in table form."""
		skip = max(0, min(skip, len(self.data)))

		if limit < 0:
			limit = len(self.data)
		limit = min(limit, len(self.data) - skip)

		lines = list()
		for data in self.data[skip:skip + limit]:
			if data['data'] == '':
				processed_data = (
					'{timestamp:>12d}ms '
					'{tag:<12s} ').format(
					timestamp=data['timestamp'],
					tag=data['tag'])

			else:
				processed_data = (
					'{timestamp:>12d}ms '
					'{tag:<12s} '
					'{data:#010b} '
					'      {data:#04X} '
					'       {data:>03d} '
					'{data:>10c} ').format(
					timestamp=data['timestamp'],
					tag=data['tag'],
					data=int(data['data'], 16))

			lines.append(processed_data)

		return lines

	def length(self):
		"""Number of rows of data."""
		return len(self.data)


class PinSignal(Signal):
	"""Pin signals. Shows HIGH or LOW as well as transistions."""

	def __init__(self, name, pin):
		"""Init."""
		super(PinSignal, self).__init__(UDP_PORT + pin, name)
		self.line = self.pin = pin

	def present_timeline(self, skip=0, limit=-1):
		"""Return the most recent cached data upto the limit.
		
		For the skip value 0 is most recent.
		"""
		skip = min(skip, len(self.data))

		if limit < 0:
			limit = len(self.data)
		limit = min(limit, len(self.data) - skip)

		self.new_data = False
		label = u'{name}({pin})'.format(name=self.name, pin=self.pin)

		data = list(reversed(self.data))[skip:skip + limit]
		data = [d['data'] for d in reversed(data)]
		
		return u'{label:<10} {data}'.format(label=label, data=u''.join(data))


def timeline(timeline_win, timeline_signals):
	"""Pump the timeline"""
	for s in timeline_signals:
		hy, hx = timeline_win.getmaxyx()
		text = encoder(s.present_timeline(0, hx - 20))
		try:
			timeline_win.addstr(s.line_hint() - 1, 1, text)
		except Exception:

			curses.nocbreak()
			curses.echo()
			curses.endwin()
			print text
			print s.line_hint()
			raise
	timeline_win.refresh()


def details(detail_win, detail_signal):
	s = detail_signal

	hy, hx = detail_win.getmaxyx()
	hy -= 3
	offset = max(0, s.length() - hy)
	limit = hy

	table = s.present_table(offset, limit)

	for row_num, row_text in enumerate(table):
		detail_win.move(row_num + 2, 1)
		detail_win.clrtoeol()
		detail_win.addstr(row_num + 2, 1, row_text)
		# Status.write('offset = {}, limit = {}, len = {}, row_num = {}'.format(offset, limit, s.length(), row_num))

	detail_win.border()
	detail_win.refresh()


class Status(object):

	window = None

	@classmethod
	def set_window(cls, window):
		cls.window = window

	@classmethod
	def write(cls, text):
		cls.window.move(2, 1)
		cls.window.clrtoeol()
		cls.window.addstr(2, 1, encoder(text))
		cls.window.border()
		cls.window.refresh()


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
	curses.curs_set(False)
	
	try:
		terminal_width, terminal_height = get_terminal_size()
		
		header_height = 4
		timeline_height = 8
		status_height = 4
		details_height = terminal_height - header_height - timeline_height - status_height
		
		header_start = 0
		timeline_start = header_height
		details_start = timeline_start + timeline_height
		status_start = details_start + details_height

		header_win = curses.newwin(header_height, terminal_width, header_start, 0)
		timeline_win = curses.newwin(timeline_height, terminal_width, timeline_start, 0)
		details_win = curses.newwin(details_height, terminal_width, details_start, 0)
		status_win = curses.newwin(status_height, terminal_width, status_start, 0)
		Status.set_window(status_win)

		timeline_win.border()
		details_win.border()
		status_win.border()

		timeline_win.refresh()

		details_header = '{:>14} {:>12s} {:>10} {:>10} {:>10} {:>10}'.format(
			'Timestamp',
			'Tag',
			'Binary',
			'Hex',
			'Decimal',
			'ASCII'
		)
		details_win.addstr(1, 1, encoder(details_header))
		details_win.refresh()

		status_win.addstr(1, 1, encoder(u'Commands: (P)ause, (←) Scrub Left, (→) Scrub Right, (Q)uit'))
		status_win.refresh()
		
		data_signal = DataSignal('DATA', 101, 6)  # The data in hex
		timeline_signals = [
			PinSignal('SDA', 2),  # SDA signal pin
			PinSignal('SCL', 3),  # SCL signal pin
			DataSignal('BIT   ', 100, 4),  # The bits as they get read
			data_signal,
		]

		# Start listening
		Signal.listen()

		# Header
		header_win.addstr(0, 0, '*' * terminal_width)
		header_win.addstr(1, 0, 'I2C Watcher')
		header_win.addstr(2, 0, '*' * terminal_width)
		header_win.refresh()

		# Input
		status_win.keypad(1)
		status_win.nodelay(True)

		# Update the data streams
		pause = False
		while True:
			timeline(timeline_win, timeline_signals)
			details(details_win, data_signal)
			try:
				c = status_win.getch()
			except curses.ERR:
				c = None

			if c in ('q', 'Q'):
				return
			elif c in ('p', 'P'):
				pause = not pause
				for s in timeline_signals:
					s.pause(pause)
			elif c in (curses.KEY_LEFT, curses.KEY_RIGHT):
				return
			elif c in (curses.KEY_UP, curses.KEY_DOWN):
				return
			else:
				if c is not None:
					status_win.addstr(2, 0, encoder(unicode(c)))

	finally:
		stdscr.keypad(0)
		curses.curs_set(True)
		curses.nocbreak()
		curses.echo()
		curses.endwin()

if __name__ == '__main__':
	main()
