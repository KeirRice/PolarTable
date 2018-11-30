#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time
import threading
import socket
import json
import Queue

SERVER_IP = '192.168.1.2'
UDP_PORT = 6000

# Simulation speed.
SIMULATION_SPEED = 0.01
SAMPLE_SPEED = SIMULATION_SPEED / 10.0

START_TIME = time.time()

NUDGE = 'nudge'
DETAIL = 'detail'

def millis():
	"""Milli seconds since script start."""
	return int((time.time() - START_TIME) * 1000)


def set_server(server):
	global SERVER_IP
	global UDP_PORT
	SERVER_IP, UDP_PORT = server[0], int(server[1])


# Client Side

class Probe(threading.Thread):
	"""Monitor the levels of a given PIN."""

	def __init__(self, port_offset):
		"""Init."""
		super(Probe, self).__init__()
		
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.address = (SERVER_IP, UDP_PORT + port_offset)

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
					self.broadcast(' ')
			else:
				try:
					while True:
						payload = self.char.get_nowait()
						self.broadcast(*payload)
				except Queue.Empty:
					pass
			time.sleep(SAMPLE_SPEED)


class ContextProbe(Probe):
	"""Instrument code with this class to send data back to the monitor."""

	pass
