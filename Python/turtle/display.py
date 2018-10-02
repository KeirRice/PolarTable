#! /usr/bin/python
# -*- coding: utf-8 -*-

# Imports ####################################################################

import Tkinter as tk
import threading
import time
from Queue import LifoQueue, Empty

from pen import Pen, debug_pen
from turtle import Turtle
from constants import FAST
from units import DisplayPosition, Polar, Cartesian, TkSpace

turtle = Turtle()


# Class ######################################################################

class Display(object):
	"""Abstraction that converts our polar motor signals to pixels on the screen."""

	c = DisplayPosition(0, 0)
	p = Polar(0, 0)
	c_pixels = DisplayPosition(0, 0)
	c_pixels_whole = DisplayPosition(0, 0)

	offset = Cartesian(0, 0)

	hud = None
	canvas = None

	debug_enabled = True
	debug_pen = None

	class HUD(threading.Thread):
		"""HUD manager."""

		def __init__(self):
			"""Setup the HUD elements."""
			super(Display.HUD, self).__init__()
			self.canvas = turtle.screen.getcanvas()
			
			self.hud_lines = dict()
			self.queues = dict()

			self.next_pos = TkSpace(-102.0, -98.0)

			self.stop = False
			self.daemon = True

		def __setitem__(self, queue_name, value):
			"""Set text on HUD item.

			Use a queue to make the value avaiable when we refresh the HUD.
			Passing new keys into here will create new HUD lines.
			"""
			if queue_name not in self.queues:
				self.queues[queue_name] = LifoQueue()
			if queue_name not in self.hud_lines:
				self.hud_lines[queue_name] = self.canvas.create_text(*self.next_pos, text='', fill='grey', anchor=tk.SW)
				self.next_pos.y += 5.0

			try:
				while True:
					self.queues[queue_name].get_nowait()
			except Empty:
				pass
			self.queues[queue_name].put_nowait(value)

		def run(self):
			"""Update the HUD text."""
			while not self.stop:
				for queue_name, q in self.queues.iteritems():
					try:
						self.canvas.itemconfig(self.hud_lines[queue_name], text=q.get_nowait())
					except Empty:
						pass
				time.sleep(0.5)

	def __init__(self):
		"""Set the HUD elements."""
		if Display.hud is None:
			Display.hud = Display.HUD()
			Display.hud.start()

		if Display.canvas is None:
			Display.canvas = turtle.screen.getcanvas()
			Display.debug_pen = Pen()

		self.pen = Pen()

	def setpos(self, polar):
		"""Set the position of of the polar coordinates to pixels on the screen."""
		self.p = polar
		self.c = self.p.cartesian()
		new_pixel_position = self.c.pixels().trunc(factor=10)
		if self.c_pixels != new_pixel_position:

			# a = new_pixel_position * 0.86666666666666666666666
			# self.pen.down()
			# self.pen.setpos(self.c)
			# self.pen.up()

			# TODO: What is going on here?!?
			# Some how my converion from mm to pixels that works everywhere else doesn't here.
			turtle.setpos(new_pixel_position * 0.86666666666666666 * 0.86666666666666666666666)
			if not FAST:
				turtle.setheading(polar.t.degrees())
			self.c_pixels = new_pixel_position

		# Post values to the HUD
		self.hud['hud_polar'] = self.p
		self.hud['hud_cartesian'] = self.c

	@classmethod
	def set_offset(cls, offset):
		"""Apply an offset to align with the real drawings."""
		cls.offset = offset

	@classmethod
	def debug_point(cls, point, color):
		"""Draw a colored dot."""
		if cls.debug_enabled:
			pen = Display.debug_pen
			pen.color = color
			if cls.offset:
				point = point + cls.offset

			pen.setpos(point)
			pen.down()
			pen.dot(5, color)
			pen.up()
	
	@classmethod
	def debug_line(cls, start_point, end_point, color='grey'):
		"""Draw a line."""
		if cls.debug_enabled:
			pen = Display.debug_pen
			if cls.offset:
				start_point = start_point + cls.offset
				end_point = end_point + cls.offset

			pen.down()
			pen.line(start_point, end_point)
			pen.up()

	@classmethod
	def debug_circle(cls, center_point, radius, color='grey'):
		"""Draw a line."""
		if cls.debug_enabled:
			pen = Display.debug_pen
			if cls.offset:
				center_point = center_point + cls.offset

			pen.down()
			pen.circle(center_point, radius, color)
			pen.up()

	@classmethod
	def debug_text(cls, point, text):
		"""Write some text."""
		pass
		# if cls.debug_enabled:
		# 	with DebugPen('grey', 1) as pen:
		# 		pen.setpos(point.pixels() * 0.86666666666666666 * 0.86666666666666666)
		# 		pen.write(text)
	
	@classmethod
	def debug_clear(cls):
		"""Clear all debug drawing."""
		if cls.debug_enabled:
			debug_pen.clear()
