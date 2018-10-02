#! /usr/bin/python
# -*- coding: utf-8 -*-

# Imports ####################################################################

import math

from display import Display
import turtle
from constants import CCW, CW
from shapes import Spiral, Koru, Transform
from units import Polar, Cartesian, Steps, Radians


# Class ######################################################################

class PolarPlot(object):
	"""Plot shapes in a polar frame of refrence."""

	def __init__(self, controller):
		"""Init."""
		self.display = Display()
		self.c = controller

		self.polar = Polar(0.0, 0.0)
		self.polar_error = Polar(0.0, 0.0)

		self.cart = Cartesian(0.0, 0.0)
		self.cart_error = Cartesian(0.0, 0.0)
		self.grid_size = self.c.grid_size

		self.direction = CCW

	def pos(self):
		"""Return the current position."""
		return self.polar

	def setpos(self, polar):
		"""Set to an absolute position."""
		# print 'PolarPlot.setpos({})'.format(polar)
		# Display.debug_line(self.polar.cartesian(), polar.cartesian(), 'yellow')
		r = polar.r - self.polar.r

		def shortest_angle(a, b):
			delta = (b - a) % math.tau
			if delta < 0:
				if abs(delta + math.tau) < abs(delta):
					delta = delta + math.tau
			else:
				if abs(delta - math.tau) < abs(delta):
					delta = delta - math.tau
			return delta
		delta_t = shortest_angle(self.polar.t, polar.t)

		self.move(Polar(r, delta_t))

	def move(self, polar):
		"""Move relative to our current position."""
		# print 'PolarPlot.move({})'.format(polar)
		delta = polar
		if delta == Polar(0, 0):
			return

		steps_number = (delta / self.grid_size).max_abs_value()
		delta_step = delta / float(steps_number)
		current = self.polar_error
		for i in range(int(steps_number)):
			current += delta_step
			whole = Steps(current // self.grid_size)
			current = current % self.grid_size
			if whole.r == 0 and whole.t == 0:
				continue
				
			for movement in self.c.step(whole):
				if movement.r.is_boundry:
					delta_step.r = 0.0

				self.polar = self.polar + movement
				self.display.setpos(self.polar)

		self.polar_error = current
		
	def flower(self, petals, amplitude):
		"""Flower."""
		n = float(petals) / 2.0
		if petals % 2 == 0:
			d = 1
		else:
			d = 0.5

		turtle.up()
		self.setpos(Polar(0.0, 0.0))
		turtle.down()

		self.rhodonea(n, d, amplitude)

	def rhodonea(self, n, d, amplitude):
		"""Drawing for the full set of rhodonea curves."""
		# amplitude *= 100
		# target_step_size = Millimeters(0.1)
		# circumference = Millimeters(math.tau * amplitude)
		# step_count = int(math.ceil(circumference / target_step_size))
		# # step_size_mm = circumference / step_count
		# step_size_rad = Radiansmath.tau / step_count
		step_count = 100

		k = float(n) / float(d)
		theta_range = Radians(math.tau) * d

		turtle.up()
		self.setpos(Polar(0.0, self.polar_error.t))

		turtle.down()
		step_size_rad = theta_range / float(step_count)
		for step in range(step_count + 1):
			t = step * step_size_rad
			r = amplitude * math.sin(k * t)
			self.setpos(Polar(r, t))
		turtle.up()

	def spiral(self, start_r, end_r, pitch_count):
		"""A constant spiral."""
		turtle.up()
		start = Polar(start_r, self.polar_error.t)
		self.setpos(start)

		turtle.down()
		for step in Spiral(start_r, end_r, pitch_count, self.polar_error.t).functor():
			self.setpos(step)
		turtle.up()

	def koru(self):
		"""A constant spiral."""
		turtle.up()
		start = Polar(0.0, 0.0)
		self.setpos(start)

		print 'koru'

		koru = Transform(Koru(), Cartesian(0.0, 150.0))

		turtle.down()
		for step in koru:
			self.setpos(step)
		turtle.up()