#! /usr/bin/python
# -*- coding: utf-8 -*-

# Imports ####################################################################

import math

import spirograph
from display import Display
import turtle
from constants import CCW, CW
from utils import arc_center, calculate_arc_length, calculate_angle
from shapes import Spiral
from units import Cartesian, CartesianResolution, Millimeters, Steps, Unit, Radians, Polar


# Class ######################################################################

class CartesianPlot(object):
	"""Plot shapes in a cartesian frame of refrence."""

	def __init__(self, controller):
		"""Init."""
		self.c = controller

		self._previous_cart = Cartesian(0.0, 0.0)
		self._cart = Cartesian(0.0, 0.0)
		self.cart_error = Cartesian(0.0, 0.0)
		self.grid_size = CartesianResolution(Millimeters(0.01), Millimeters(0.01))

		self.offset_to_origin = Cartesian(0.0, 0.0)

	def set_feedrate(self, value):
		pass
		# TODO

	def set_origin(self, offset_to_origin):
		"""Transform the coordinate space."""
		self.offset_to_origin += offset_to_origin
		Display.set_offset = offset_to_origin

	def set_cart(self, value):
		"""Setter for cart that keeps a copy of the last value."""
		if self._cart != value:
			self._previous_cart = self.cart
			self._cart = value

	def get_cart(self):
		"""Getter for cart."""
		return self._cart

	cart = property(get_cart, set_cart)

	def get_previous_cart(self):
		"""Return the previous value of cart."""
		return self._previous_cart

	previous_cart = property(get_cart)

	def to_relative(self, position, base=None):
		"""Transform an absolute position to a relative one."""
		if base is None:
			base = self.cart
		return position - base

	def to_absolute(self, position, base=None):
		"""Transform a relative position to an absolute one."""
		if base is None:
			base = self.cart
		return position + base

	def __str__(self):
		"""String."""
		return u'Controller(<{}>)'.format(self.cart)

	def pos(self):
		"""Current position."""
		return self.c.pos().cartesian()

	def setpos_fast(self, position):
		"""Set our position using absolute cartesian coordinates."""
		# Display.debug_line(self.cart, cart, 'orange')
		self.move_fast(self.to_relative(position))

	def move_fast(self, delta):
		"""Set our position using relative cartesian coordinates."""
		if delta == Cartesian(0, 0):
			return

		current = self.cart_error + delta
		whole = Steps(current // self.grid_size, cartesian=True)
		current = current % self.grid_size
		
		self.cart = self.cart + (self.grid_size * whole)
		self.set_controller_pos(self.cart)
		self.cart_error = current

	def setpos(self, position):
		"""Set our position using absolute cartesian coordinates."""
		# Display.debug_line(self.cart, cart, 'orange')
		self.move(self.to_relative(position))

	def move(self, delta):
		"""Set our position using relative cartesian coordinates."""
		if delta == Cartesian(0, 0):
			return

		steps_number = delta.max_abs_value()
		delta_step = delta / steps_number

		current = self.cart_error
		for r in range(int(steps_number) + 1):
			current += delta_step

			whole = Steps(current // self.grid_size, cartesian=True)
			current = current % self.grid_size
			
			# print '\t', r + 1, 'in', int(steps_number) + 1, whole
			self.cart = self.cart + (self.grid_size * whole)
			self.set_controller_pos(self.cart)

		self.cart_error = current

	def set_controller_pos(self, cart):
		"""Set the polar plot coordinates to match our coordinates."""
		return self.c.setpos((cart + self.offset_to_origin).polar())

	def draw_line(self, point1, point2):
		"""Draw straight line between absolute points using catesian interpolation."""
		if point1 is None:
			point1 = self.cart

		print 'draw_line {} => {}'.format(point1, point2)

		Display.debug_line(point1, point2)
		Display.debug_point(point1, 'green')
		Display.debug_point(point2, 'red')
		Display.debug_text(point2, point2)

		# turtle.up()
		self.setpos(point1)
		# turtle.down()
		self.setpos(point2)
		# turtle.up()

	def draw_lines(self, *points):
		"""Draw straight line between absolute points using catesian interpolation."""
		last_point = points[0]
		for point in points[1:]:
			self.draw_line(last_point, point)
			last_point = point

	def draw_polar_line(self, *args):
		"""Draw line between absolute points using polor interpolation."""
		if len(args) == 4:
			start_point = Cartesian(*args[:2])
			end_point = Cartesian(*args[2:])
		elif len(args) == 2:
			start_point, end_point = args
		else:
			raise RuntimeError('Incorrect arguments.')
		print 'draw_polar_line {} => {}'.format(start_point, end_point)

		# turtle.up()
		self.set_controller_pos(start_point)
		# turtle.down()
		self.set_controller_pos(end_point)
		# turtle.up()

	def draw_circle(self, center_point, radius):
		"""Draw a circle with the given center and radius."""
		print 'draw_circle centered on {} radius {}'.format(center_point, radius)
		circumference = float(radius) * math.tau
		edges_per_mm = 0.5
		edge_count = circumference * float(edges_per_mm)
		edge_count = int(math.ceil(edge_count))

		# Set the starting angle for the circle to be close to our current location.
		if center_point == Cartesian(0.0, 0.0):
			start_angle = 0.0
		else:
			base = center_point - self.cart
			start_angle = Cartesian(1.0, 0.0).angle(base)
			if self.cart.x < center_point.x or self.cart.y < center_point.y:
				start_angle += math.pi

		self.draw_polygon(center_point, radius, edge_count, start_angle)

	def draw_arc(self, center_point, radius, start_angle, end_angle, direction=CCW, arc_angle=None):
		"""Draw an arc."""
		print 'draw_arc centered on {} with radius of {}, between {} and {}'.format(
			center_point, radius, start_angle, end_angle)
		
		if arc_angle is None:
			arc_angle = abs(end_angle - start_angle)
		arc_length = radius * Unit(arc_angle)
		edges_per_mm = 0.5
		edge_count = arc_length * float(edges_per_mm)
		edge_count = max(3, int(math.ceil(edge_count)))

		return self.draw_polygon_arc(
			center_point, radius, (edge_count, arc_angle), start_angle, end_angle, direction)

	def draw_2point_arc(self, point1, point2, radius, direction):
		"""Draw an arc given two points and a radius."""
		if point1 is None:
			point1 = self.cart
		
		Display.debug_circle(point1, radius)
		Display.debug_circle(point2, radius)
		if point1 == point2:
			if direction == CW:
				center_point = Cartesian(point1.x + radius, point1.y)
				start_angle = Radians(math.pi)
			else:
				center_point = Cartesian(point1.x - radius, point1.y)
				start_angle = Radians(0.0)
			end_angle = Radians(start_angle + math.tau)
			Display.debug_point(center_point, 'purple')

		else:
			center_point = arc_center(point1, point2, radius, direction)
			Display.debug_point(center_point, 'blue')

			start_angle = calculate_angle(center_point, point1) % math.tau
			# TODO: This needs direct to choose end angle.
			end_angle = calculate_angle(center_point, point2) % math.tau

		self.draw_multi_point_arc(point1, point2, center_point, radius, start_angle, end_angle)

	def draw_3point_arc(self, point1, point2, relative_center_point, direction):
		"""Draw an arc given three points."""
		if point1 is None:
			point1 = self.cart

		center_point = point1 + relative_center_point
		radius = Millimeters(relative_center_point.length())

		start_angle = calculate_angle(center_point, point1) % math.tau
		end_angle = calculate_angle(center_point, point2) % math.tau

		self.draw_multi_point_arc(point1, point2, center_point, radius, start_angle, end_angle, direction)

	def draw_multi_point_arc(self, point1, point2, center_point, radius, start_angle, end_angle, direction=CCW):
		"""Draw an arc given three points."""
		if point1 is None:
			point1 = self.cart

		Display.debug_point(point1, 'green')
		Display.debug_point(point2, 'red')

		# turtle.up()
		self.setpos(point1)

		arc_length, arc_angle = calculate_arc_length(center_point, point1, point2, radius, direction)
		
		# turtle.down()
		self.draw_arc(center_point, radius, start_angle, end_angle, direction, arc_angle)
		# turtle.up()

	def draw_polygon(self, center_point, radius, edge_count, start_angle=None):
		"""Draw a polygon."""
		if start_angle is None:
			# Set the starting angle for the circle to be close to our current location.
			if center_point == Cartesian(0.0, 0.0):
				start_angle = 0.0
			else:
				base = center_point - self.cart
				start_angle = Cartesian(1.0, 0.0).angle(base)
				if self.cart.x < center_point.x or self.cart.y < center_point.y:
					start_angle += math.pi

		print 'draw_polygon centered on {} radius {} edges {}'.format(center_point, radius, edge_count)
		end_angle = Radians(start_angle + (math.tau))
		direction = CCW
		return self.draw_polygon_arc(
			center_point, radius, edge_count, start_angle, end_angle, direction)

	def draw_polygon_arc(self, center_point, radius, edge_count, start_angle, end_angle, direction):
		"""Draw a polygon."""
		if isinstance(edge_count, tuple):
			edge_count, arc_angle = edge_count
		else:
			arc_angle = end_angle - start_angle

		print 'draw_polygon_arc with {} edges, centered on {} with radius of {}, between {} and {}'.format(
			edge_count, center_point, radius, start_angle, end_angle)
		
		edge_count = int(math.ceil(edge_count))
		if edge_count < 3:
			raise ValueError('draw_polygon needs a minimum of 3 edges.')

		Display.debug_circle(center_point, radius)
		Display.debug_point(center_point, 'green')

		delta_step = Unit(arc_angle / float(edge_count))
		if direction == CW:
			delta_step = -abs(delta_step)
		elif direction == CCW:
			delta_step = abs(delta_step)

		polar = Polar(radius, start_angle)
		
		turtle.up()
		self.setpos(center_point + polar.cartesian())
		Display.debug_point(center_point + polar.cartesian(), 'pink')
		
		turtle.down()
		# direction = sign(delta_step)
		for i in range(int(edge_count)):
			polar.t += delta_step
			self.setpos(center_point + polar.cartesian())
			# if (direction == -1 and polar.t < end_angle) or (direction == 1 and polar.t > end_angle):
			# 	break
		# turtle.up()

	def spirograph(self):

		pen1 = spirograph.Pen(100, 'red')
		pen2 = spirograph.Pen(50, 'red')
		pen3 = spirograph.Pen(25, 'green')
		a = spirograph.Cog(448)
		b = spirograph.Cog(210)
		a + b
		b + pen1
		b + pen2
		b + pen3

		turtle.up()
		a.start(Radians(0.0))
		self.setpos(pen1.position())

		turtle.down()
		for step in a.steps():
			self.setpos(pen1.position())
			# Display.debug_point(pen1.position(), 'red')
			# Display.debug_point(pen2.position(), 'blue')
			# Display.debug_point(pen3.position(), 'green')
		turtle.up()

	# def spiral(self, radius_start, radius_end, pitch_count):
	# 	"""A constant spiral."""
	# 	radius_range = radius_end - radius_start
	# 	radius_step = float(radius_range) / pitch_count
	#
	# 	turtle.up()
	# 	start = Polar(radius_start, self.polar_error.t)
	# 	self.setpos(start)
	#
	# 	turtle.down()
	# 	for i in range(int(pitch_count)):
	# 		rel_move = Polar(radius_step, math.tau)
	# 		self.move(rel_move)
	# 	turtle.up()
	
	# def spiral(self, center, start_r, end_r, pitch_count):
	# 	"""A constant spiral."""
	# 	turtle.up()
	# 	start = Polar(start_r, self.polar_error.t)
	# 	self.setpos(start)
	#
	# 	turtle.down()
	# 	for step in Spiral(start_r, end_r, pitch_count, self.polar_error.t):
	# 		self.setpos(center + step.cartesian())
	# 	turtle.up()