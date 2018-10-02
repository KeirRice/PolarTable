#! /usr/bin/python
# -*- coding: utf-8 -*-

# Imports ####################################################################

import math

import turtle as turtle_global
from turtle import Turtle
from units import Cartesian, Millimeters, Distance


debug_pen = Turtle()
debug_pen.speed(0)
debug_pen.pen(shown=False, pencolor='red', pensize=3, speed=0)
debug_pen.hideturtle()

# Class ######################################################################

class Pen(object):
	"""Draw directly on the canvas."""

	canvas = None

	def __init__(self):
		"""Set the HUD elements."""
		if Pen.canvas is None:
			Pen.canvas = turtle_global.getscreen().getcanvas()
		
		self._transform = Cartesian(0.866666666, -0.866666666).pixels()

		self.pen_down = False
		self.color = 'black'
		self._internal_position = Cartesian(0.0, 0.0)

	def get_internal_position(self):
		"""Return position in tkinter space."""
		return self._internal_position

	def set_internal_position(self, position):
		"""Set a position for this pen."""
		if isinstance(position, Cartesian):
			position = position.pixels()
		position = position * self._transform
		self._internal_position = position

	position = property(get_internal_position, set_internal_position)

	def getpos(self, units=None):
		"""Get the position of the pen."""
		if units is None:
			units = Millimeters
		return units(self._internal_position / self._transform)

	def up(self):
		"""Pen up."""
		self.pen_down = False

	def down(self):
		"""Pen down."""
		self.pen_down = True

	def setpos(self, position):
		"""Move to position and dot."""
		self.position = position
		if self.pen_down:
			p = self.position
			self.canvas.create_oval(p.x, p.y, p.x, p.y, fill=self.color)
			self.refresh()

	def dot(self, size=1, color=None):
		"""Splat a dot."""
		if isinstance(size, Distance):
			size = size.pixels()
		size = math.floor(float(size) * 0.5)
		if color is None:
			color = self.color
		state = self.pen_down
		self.down()
		p = self.position
		self.canvas.create_oval(
			p.x - size, p.y - size, p.x + size, p.y + size, fill=color, outline=color)
		self.refresh()
		self.pen_down = state

	def line(self, point1, point2):
		"""Draw a line."""
		p1 = point1.pixels() * self._transform
		p2 = point2.pixels() * self._transform
		state = self.pen_down
		self.down()
		self.canvas.create_line(p1.x, p1.y, p2.x, p2.y)
		self._internal_position = p2
		self.refresh()
		self.pen_down = state

	def circle(self, center_point, radius, color=None):
		"""Draw a circle."""
		if color is None:
			color = self.color
		p = center_point.pixels() * self._transform
		r = radius.pixels() * self._transform.x
		state = self.pen_down
		self.down()
		self.canvas.create_oval(
			p.x - r, p.y - r, p.x + r, p.y + r, outline=color)
		self.refresh()
		self.pen_down = state

	def refresh(self):
		"""Update the canvas."""
		self.canvas.update_idletasks()

	def transform(self, value):
		"""Scale to the canvas."""
		return value * self._transform
	
	
class DebugPen(object):
	"""Wrapper for working with the debug pen."""

	def __init__(self, color='grey', size=1):
		"""Save old values and ready new ones."""
		self.new_color = color
		self.new_size = size
		self.orignal_color = debug_pen.pencolor()
		self.orignal_size = debug_pen.pensize()

	def __enter__(self):
		"""Set new values."""
		debug_pen.pencolor(self.new_color)
		debug_pen.pensize(self.new_size)
		return debug_pen

	def __exit__(self, *args):
		"""Back to old values."""
		debug_pen.pencolor(self.orignal_color)
		debug_pen.pensize(self.orignal_size)
