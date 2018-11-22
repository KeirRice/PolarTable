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

	def broadcast(self, char, tag='', timestamp=None):
		"""Send char out over UDP, hopfully to a listening monitor."""
		outgoing = json.dumps({
			'data': char,
			'tag': tag.encode('utf-8'),
			'timestamp': timestamp or millis(),
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
		self.char.put((char, tag, millis()))

	def run(self):
		"""Main loop catches data and broadcasts it."""
		while True:
			if self.char.empty():
				if self.compact is False:
					self.broadcast(' ', '', millis())
			else:
				try:
					while True:
						data = self.char.get_nowait()
						self.broadcast(*data)
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
		self.selection_index = -1

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
			incomming['arrived'] = millis()
			self.data.append(incomming)
	
	def present_timeline(self, skip=0, limit=-1):
		"""Return the most recent cached data upto the width."""
		return NotImplemented

	def present_table(self, skip=0, limit=-1):
		"""Return the most recent cached data upto the width."""
		return NotImplemented

	def pause(self, pause):
		"""Pause data capture?"""
		pass

	def length(self):
		"""Number of rows of data."""
		return len(self.data)

	def index_to_timestamp(self, index):
		"""Return the timestamp for the index."""
		return self.data[index]['timestamp']

	def timestamp_to_index(self, timestamp):
		"""Return the time stamp for the index."""
		for i, d in enumerate(self.data):
			if d['timestamp'] > timestamp:
				return i - 1
		raise LookupError('No timestamp found.')


class DataSignal(Signal):
	"""Data signals. Anything text based."""

	def __init__(self, name, port_offset, line):
		"""Init."""
		super(DataSignal, self).__init__(UDP_PORT + port_offset, name)
		self.line = line
		self.seperator = u''
		self.timeline_format_string = u'{}'
		self.timeline_format_width = 1

	def set_seperator(self, seperator):
		"""If the out put needs a seperator character set it here."""
		self.seperator = seperator
	
	def set_timeline_format_string(self, format_string, width):
		"""How are we presenting the data and how wide will if be."""
		self.timeline_format_string = format_string
		self.timeline_format_width = width

	def present_table(self, skip=0, limit=-1):
		"""Return the most recent cached data in table form."""
		skip = clamp(0, skip, len(self.data) - 1)

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
					data=data['data'])

			lines.append(processed_data)

		return lines

	def present_timeline(self, skip=0, limit=-1):
		""""""
		skip = clamp(0, skip, self.length() - 1)
		
		data = list()
		counter = 0
		for d in self.data[skip:]:
			if d['data'] != '':
				data.append(self.timeline_format_string.format(d['data']))
				counter += 1
			if counter >= limit:
				break

		if limit < 0:
			limit = len(data) - 1
		limit = clamp(0, limit, len(data) - 1)

		label = u'{label:<10}'.format(
			label=self.name,
		)
		data_string = self.seperator.join(data)
		return label, data_string

	def width_to_limit(self, display_width):
		"""Return the max limit of data items that will fit in display_width."""
		label_width = 11
		timeline_width = display_width - label_width
		max_data_item_count = timeline_width // self.timeline_format_width
		return max_data_item_count


class PinSignal(Signal):
	"""Pin signals. Shows HIGH or LOW as well as transistions."""

	def __init__(self, name, pin):
		"""Init."""
		super(PinSignal, self).__init__(UDP_PORT + pin, name)
		self.line = self.pin = pin

	def present_timeline(self, skip=0, limit=-1):
		"""Return the most recent cached data upto the limit."""
		skip = clamp(0, skip, self.length() - 1)
		limit = clamp(0, limit, self.length() - 1 - skip)

		label = u'{name}({pin})'.format(name=self.name, pin=self.pin)

		data = [d['data'] for d in self.data[skip:skip + limit]]
		
		label = u'{label:<10}'.format(
			label=self.name,
		)
		data_string = u'{data}'.format(
			data=u''.join(data)
		)
		return label, data_string

	def width_to_limit(self, display_width):
		"""Return the max limit of data items that will fit in display_width."""
		label_width = 11
		timeline_width = display_width - label_width
		data_format_width = 1
		max_data_item_count = timeline_width // data_format_width
		return max_data_item_count


class Window(object):
	"""Curses Window."""

	status_window = None

	def __init__(self, stdscr):
		"""Init."""
		self.stdscr = stdscr

		self.quit = False
		self.paused = False
		self.selection_index = -1
		self.selection_timestamp = -1

		self.detail_length = 0
		self.detail_data_window = None

		self.timeline_data_window = dict()

		self.sections = list()

		self.header_layout = None
		self.timeline_layout = None
		self.details_layout = None
		self.status_layout = None

		self.header_win = None
		self.timeline_win = None
		self.details_win = None
		self.status_win = None

		self.header_win_border = (0, 0, 0, 0, 0, 0, curses.ACS_LTEE, curses.ACS_RTEE)
		self.timeline_win_border = (0, 0, ' ', 0, curses.ACS_VLINE, curses.ACS_VLINE, curses.ACS_VLINE, curses.ACS_VLINE)
		self.details_win_border = (0, 0, ' ', 0, curses.ACS_VLINE, curses.ACS_VLINE, 0, 0)
		self.status_win_border = (0, 0, 0, 0, 0, 0, 0, 0)
		
		self.windows = list()

		self.height, self.width = get_terminal_size()

		self.create()
		self.content()
		self.border()
		self.refresh()

	def create(self):
		"""Create the windows."""
		self.layout()
		self.header_win = curses.newwin(*self.header_layout)
		self.windows.append(self.header_win)
		self.timeline_win = curses.newwin(*self.timeline_layout)
		self.windows.append(self.timeline_win)
		self.details_win = curses.newwin(*self.details_layout)
		self.windows.append(self.details_win)
		self.status_win = curses.newwin(*self.status_layout)
		self.windows.append(self.status_win)
		Window.status_window = self.status_win

	def layout(self):
		"""Calculate the sizes and locations of the windows."""
		self.width, self.height = get_terminal_size()
		
		header_height = 3
		timeline_height = 8
		status_height = 4
		details_height = self.height - header_height - timeline_height - status_height
		
		header_start = 0
		timeline_start = header_height
		details_start = timeline_start + timeline_height
		status_start = details_start + details_height

		self.header_layout = (header_height, self.width, header_start, 0)
		self.timeline_layout = (timeline_height, self.width, timeline_start, 0)
		self.details_layout = (details_height, self.width, details_start, 0)
		self.status_layout = (status_height, self.width, status_start, 0)

	def resize(self):
		"""Re-apply the layouts to the windows."""
		self.layout()

		for window in self.windows:
			window.erase()

		self.header_win.resize(self.header_layout[0], self.header_layout[1])
		self.timeline_win.resize(self.timeline_layout[0], self.timeline_layout[1])
		self.details_win.resize(self.details_layout[0], self.details_layout[1])
		self.status_win.resize(self.status_layout[0], self.status_layout[1])

		self.header_win.mvwin(self.header_layout[2], self.header_layout[3])
		self.timeline_win.mvwin(self.timeline_layout[2], self.timeline_layout[3])
		self.details_win.mvwin(self.details_layout[2], self.details_layout[3])
		self.status_win.mvwin(self.status_layout[2], self.status_layout[3])

		self.content()
		self.border()
		self.refresh()

	def border(self):
		"""Add the borders."""
		self.header_win.border(*self.header_win_border)
		self.timeline_win.border(*self.timeline_win_border)
		self.details_win.border(*self.details_win_border)
		self.status_win.border(*self.status_win_border)

	def refresh(self):
		for window in self.windows:
			window.refresh()

	def content(self):
		self.header_content()
		self.details_content()
		self.status_content()
	
	def header_content(self):
		self.header_win.addstr(1, 1, 'I2C Watcher', curses.color_pair(2) | curses.A_BOLD)
		address = ip_address()
		self.header_win.addstr(1, self.width - 1 - len(address), address)
		
	def details_content(self):
		details_header = '{:>14} {:>12s} {:>10} {:>10} {:>10} {:>10}'.format(
			'Timestamp',
			'Tag',
			'Binary',
			'Hex',
			'Decimal',
			'ASCII'
		)
		self.details_win.addstr(0, 1, encoder(clip(details_header, self.width - 2)), curses.color_pair(2) | curses.A_BOLD)
	
	def status_content(self):
		help_text = u'Commands: (P)ause, (←) Scrub Left, (→) Scrub Right, (Q)uit'
		self.status_win.addstr(1, 1, encoder(clip(help_text, self.width - 2)))

	def timeline(self, timeline_signals):
		"""Pump the timeline."""
		self.timeline_win.erase()
		content_width = self.width - 2
		for i, s in enumerate(timeline_signals):

			limit = s.width_to_limit(content_width)

			if self.paused:
				offset, limit = self.timeline_data_window[i]
				if self.selection_timestamp != -1:
					offset = s.timestamp_to_index(self.selection_timestamp)
			else:
				limit = min(limit, s.length())
				offset = max(0, s.length() - limit)
				self.timeline_data_window[i] = offset, limit

			if i == 3:
				type(self).set_status(u'offset = {}, limit = {}, s.length() = {}, self.paused = {}'.format(offset, limit, s.length(), self.paused))

			label, stream = s.present_timeline(offset, limit)
			# if i == 3:
			# 	type(self).set_status(s.data)
			
			# Handle when the data samples are not exactly 1 char wide and we need to clip to prevent overflow.
			padding = content_width - len(label) - 1 - len(stream)
			if padding > 0:
				stream = (u' ' * padding) + stream

			self.timeline_win.addstr(s.line_hint(), 1, encoder(label + ' ' + stream))

		self.timeline_win.border(*self.timeline_win_border)
		self.details_content()
		self.timeline_win.refresh()

	def details(self, detail_signal):
		self.timeline_win.erase()

		if self.selection_index != -1:
			self.selection_timestamp = detail_signal.index_to_timestamp(self.selection_index)

		self.detail_length = detail_signal.length()

		if self.paused:
			offset, limit = self.detail_data_window
		else:
			hy, hx = self.details_win.getmaxyx()
			hy -= 3  # Heading + borders
			offset = max(0, detail_signal.length() - hy)
			limit = min(hy, detail_signal.length())
			self.detail_data_window = offset, limit

		table = detail_signal.present_table(offset, limit)
		
		row_number = offset
		for line_number, row_text in enumerate(table):
			self.details_win.move(line_number + 2, 1)
			self.details_win.clrtoeol()

			if self.selection_index >= 0 and row_number == self.selection_index:
				self.details_win.addstr(line_number + 2, 1, row_text, curses.color_pair(1) | curses.A_BOLD)
			else:
				self.details_win.addstr(line_number + 2, 1, row_text, curses.color_pair(2))

			row_number += 1
			
		self.details_win.border(*self.details_win_border)
		self.details_content()
		self.details_win.refresh()

	def update_selection(self, up=True):
		"""Update the selections."""
		# Move the selection in details.
		offset, limit = self.detail_data_window

		if up:
			self.selection_index -= 1
			if self.selection_index > (offset + limit):
				self.selection_index = (offset + limit)
		else:
			self.selection_index += 1
			if self.selection_index < offset:
				self.selection_index = offset
		self.selection_index = clamp(0, self.selection_index, self.detail_length - 1)

		# Bump scroll the details list.
		if self.selection_index >= (offset + limit):
			offset += 1
			self.detail_data_window = (offset, limit)
		elif self.selection_index < offset:
			offset -= 1
			self.detail_data_window = (offset, limit)

	def process_input(self):
		"""Process key presses."""
		self.status_win.nodelay(True)

		KEY_UP = 0x41
		KEY_DOWN = 0x42
		KEY_RIGHT = 0x43
		KEY_LEFT = 0x44

		try:
			c = self.status_win.getch()
		except curses.ERR:
			return

		if c in (KEY_LEFT, KEY_RIGHT):
			pass
		elif c == KEY_UP:
			self.update_selection(True)
		elif c == KEY_DOWN:
			self.update_selection(False)
		elif c == curses.KEY_RESIZE:
			self.resize()
		elif c == curses.KEY_HOME:
			return
		elif c in (ord('q'), ord('Q')):
			self.quit = True
		elif c in (ord('p'), ord('P')):
			self.paused = not self.paused
			type(self).set_status(u'pause = {}'.format(self.paused))

	@classmethod
	def set_status(cls, *args):
		"""Put text in the status line."""
		width, _ = get_terminal_size()
		text = u' '.join([unicode(a) for a in args])
		text = clip(text, width - 2)
		
		cls.status_window.move(2, 1)
		cls.status_window.clrtoeol()
		cls.status_window.addstr(2, 1, encoder(text))
		cls.status_window.border()
		cls.status_window.refresh()


def clip(text, width):
	"""Clip text to a fixed width."""
	return text[:min(len(text), width)]


def clamp(minimum, value, maximum):
	"""Clamp the value between the min and max."""
	return min(max(minimum, value), maximum)

def ip_address():
	"""Return out IP address."""
	local_ips = [ip for ip in socket.gethostbyname_ex(socket.gethostname())[2] if not ip.startswith("127.")]
	socket_message = [socket.socket(socket.AF_INET, socket.SOCK_DGRAM)]
	remote_ips = [[(s.connect(("8.8.8.8", 53)), s.getsockname()[0], s.close()) for s in socket_message][0][1]]
	return ((local_ips or remote_ips) + ["no IP found"])[0]


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
	curses.start_color()
	curses.init_pair(1, curses.COLOR_BLUE, curses.COLOR_BLACK)
	curses.init_pair(2, curses.COLOR_WHITE, curses.COLOR_BLACK)
	
	curses.noecho()
	curses.cbreak()
	curses.curs_set(False)
	
	try:
		w = Window(stdscr)

		data_signal = DataSignal('DATA', 101, 6)  # The data in hex
		timeline_signals = [
			PinSignal('SDA', 2),  # SDA signal pin
			PinSignal('SCL', 3),  # SCL signal pin
			DataSignal('BIT   ', 100, 4),  # The bits as they get read
		]

		# Start listening
		Signal.listen()

		while not w.quit:
			w.timeline(timeline_signals)
			w.details(data_signal)
			w.process_input()
			pass

	finally:
		stdscr.keypad(0)
		curses.curs_set(True)
		curses.nocbreak()
		curses.echo()
		curses.endwin()

if __name__ == '__main__':
	main()
