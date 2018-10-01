#! /usr/bin/python
# -*- coding: utf-8 -*-

from turtle import Turtle, Screen
import turtle as turtle_global
import math
import threading
from Queue import LifoQueue, Empty
import time
import Tkinter as tk
import uuid

from units import *
import spirograph

math.tau = math.pi * 2.0


def gcd(a, b):
	"""Compute the greatest common divisor of a and b."""
	while b > 0:
		a, b = b, a % b
	return a
math.gcd = gcd


def lcm(a, b):
	"""Compute the lowest common multiple of a and b."""
	return a * b / math.gcd(a, b)
math.lcm = lcm


FAST = True
FAST_SKIP_MOTORS = True

CW, CCW = 'Clockwise', 'Counter-clockwise'

window = Pixels(800)
turtle_global.getscreen().setup(width=int(window), height=int(window), startx=None, starty=None)
turtle_global.screensize(int(window) - 20, int(window) - 20, "black")
turtle_global.hideturtle()

turtle = Turtle()
screen_size = Pixels(turtle.screen.screensize()[0])
world_size = CartesianResolution(900, 900)
Distance.set_conversion_factor(screen_size, world_size.x)
TABLE_RADIUS_MM = Millimeters(450)
turtle.hideturtle()
turtle.speed(0)
turtle.color("white", "white")
turtle.begin_fill()
turtle.setpos(0, -int(TABLE_RADIUS_MM.pixels()))
turtle.circle(int(TABLE_RADIUS_MM.pixels()))
turtle.setpos(0, 0)
turtle.end_fill()
turtle.color("black", "white")
turtle.speed('fast')

turtle = Turtle()
turtle.speed(0)
turtle.screen.delay(0)

debug_pen = Turtle()
debug_pen.speed(0)
debug_pen.pen(shown=False, pencolor='red', pensize=3, speed=0)
debug_pen.hideturtle()


# class DebugPen(object):
# 	"""Wrapper for working with the debug pen."""

# 	def __init__(self, color='grey', size=1):
# 		"""Save old values and ready new ones."""
# 		self.new_color = color
# 		self.new_size = size
# 		self.orignal_color = debug_pen.pencolor()
# 		self.orignal_size = debug_pen.pensize()

# 	def __enter__(self):
# 		"""Set new values."""
# 		debug_pen.pencolor(self.new_color)
# 		debug_pen.pensize(self.new_size)
# 		return debug_pen

# 	def __exit__(self, *args):
# 		"""Back to old values."""
# 		debug_pen.pencolor(self.orignal_color)
# 		debug_pen.pensize(self.orignal_size)


class Pen(object):
	"""Draw directly on the canvas."""

	canvas = None

	def __init__(self):
		"""Set the HUD elements."""
		if Pen.canvas is None:
			Pen.canvas = turtle.screen.getcanvas()
		
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


class Display(object):
	"""Abstraction that convets our polar motor signals to pixels on the screen."""

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


class Motor(object):
	"""Motor abstraction.."""

	FORWARD = 1
	BACKWARD = -1

	def __init__(self, step_distance):
		"""Init."""
		self.counter = 0
		self.direction = self.FORWARD

		# Is the motor in full step mode?
		self.step_size = 1

		self.min_steps = None
		self.max_steps = None

		# The distance the tip will move for each step of the motor.
		self.step_distance = step_distance
		self.min_distance = None
		self.max_distance = None

		# The step size of the motor.
		self.effective_step_distance = self.step_distance * self.step_size
		self.step_time = Milliseconds(2.64).seconds()

		"""
		Step Angle (degrees) :1.8
		2 Phase
		Rated Voltage : 12V
		Rated Current : 0.33A
		Holding Torque : 2.3kg*cm
		5mm Diameter Drive Shaft
		Winding resistance: 32.6 Ω
		Winding inductance: 48 mH
		Max flux linkage: 1.8 Vs
		Maximum Detent Torque: 0.016 N.M
		Total inertia (kg.m.m): 3.5 Kg.m.m
		Total friction (kg.m/s): 4 Kg.m/s
		NEMA 16 Mounting Hole Pattern (31mm)
		Motor Width: 42mm (1.67”)


		https://www.allaboutcircuits.com/tools/stepper-motor-calculator/
		Maximum Speed: 1.89	revolutions/sec
		Minimum Time/Step: 2.64	ms
		Maximum Power: 3.96	Watts
		"""

	def distance_to_steps(self, distance):
		"""Return steps for the given distance.

		Note, returns float values that may have parital steps.
		"""
		return Unit(distance / float(self.effective_step_distance))

	def set_hard_range(self, min_distance, max_distance):
		"""Set a stop distance."""
		if min_distance is not None:
			self.min_distance = min_distance
			self.min_steps = self.distance_to_steps(min_distance)
		if max_distance is not None:
			self.max_distance = max_distance
			self.max_steps = self.distance_to_steps(max_distance)

	def set_step(self, step_size):
		"""Step size."""
		self.step_size = step_size
		self.set_hard_range(self.min_distance, self.max_distance)

	def set_direction(self, direction):
		"""Direction."""
		self.direction = direction

	def step(self):
		"""Fire a motor step."""
		target = self.counter + (self.step_size * self.direction)
		if self.min_steps is not None:
			if target < self.min_steps:
				distance = type(self.step_distance)(0.0)
				distance.is_boundry = True
				return distance
		if self.max_steps is not None:
			if target > self.max_steps:
				distance = type(self.step_distance)(0.0)
				distance.is_boundry = True
				return distance
		self.counter = target
		return self.effective_step_distance * (self.step_size * self.direction)

	def steps(self, steps):
		"""Step the radius motor."""
		self.set_direction(sign(steps))
		for i in range(int(abs(steps))):
			yield self.step()
			if not FAST:
				time.sleep(self.step_time)


class Controller(object):
	"""Abstractions to interacts with the motors."""

	def __init__(self):
		"""Init."""
		self.display = Display()

		self.r_motor = Motor(Millimeters(0.16))  # 0.16mm per step
		self.r_motor.set_hard_range(Millimeters(0.0), TABLE_RADIUS_MM)
		self.r = 0.0
		# self.r_motor.set_step(0.5)  # Half step mode

		self.t_motor = Motor(Degrees(0.05).radians())  # 0.05deg per step

		self.grid_size = PolarResolution(self.r_motor.effective_step_distance, self.t_motor.effective_step_distance)

	# def calculate_grid_size(self):
	# 	circumference = math.tau * self.r
	# 	# mm_per_raidian = circumference / Radiansmath.tau
		
	# 	target_step_mm = Millimeters(0.1)
	# 	target_steps = circumference / target_step_mm
	# 	return PolarResolution(self.r_motor.effective_step_distance, Radiansmath.tau / target_steps)

	def __str__(self):
		"""Str."""
		return u'Controller(<{}>)'.format(self.polar)

	def step_r(self, steps):
		"""Step the radius motor."""
		self.display.hud['step_r'] = 'r stepping {}'.format(steps)
		for movement in self.r_motor.steps(steps):
			self.r += movement
			yield movement
		self.display.hud['step_r'] = 'r idle'
	
	def step_t(self, steps):
		"""Step the radius motor."""
		self.display.hud['step_t'] = 't stepping {}'.format(steps)
		for movement in self.t_motor.steps(steps):
			yield movement
		self.display.hud['step_t'] = 't idle'

	if FAST_SKIP_MOTORS:
		def step(self, steps):
			"""Step the motors in sync."""
			biggest = int(max(abs(steps.r), abs(steps.t)))
			step_size = Steps(float(steps.r) / biggest, float(steps.t) / biggest)
			
			accumulation = Steps(0.0, 0.0)
			for i in range(biggest):
				accumulation += step_size
				while abs(accumulation.r) >= 1.0 or abs(accumulation.t) >= 1.0:
					this_step = Polar(0.0, 0.0)
					if abs(accumulation.r) >= 1.0:
						r_step = self.r_motor.effective_step_distance * (self.r_motor.step_size * sign(steps.r))
						this_step.r = r_step
						accumulation.r = accumulation.r - sign(steps.r)
						
					if abs(accumulation.t) >= 1.0:
						this_step.t = self.t_motor.effective_step_distance * (self.t_motor.step_size * sign(steps.t))
						accumulation.t = accumulation.t - sign(steps.t)
					yield this_step

	else:
		def step(self, steps):
			"""Step the motors in sync."""
			biggest = int(max(abs(steps.r), abs(steps.t)))
			step_size = Steps(float(steps.r) / biggest, float(steps.t) / biggest)
			
			self.r_motor.set_direction(sign(steps.r))
			self.t_motor.set_direction(sign(steps.t))

			accumulation = Steps(0.0, 0.0)
			for i in range(biggest):
				accumulation += step_size
				while abs(accumulation.r) >= 1.0 or abs(accumulation.t) >= 1.0:
					this_step = Polar(0.0, 0.0)
					if abs(accumulation.r) >= 1.0:
						r_step = self.r_motor.step()
						this_step.r = r_step
						if r_step.is_boundry:
							accumulation.r = 0.0
						else:
							accumulation.r = accumulation.r - sign(steps.r)
						
					if abs(accumulation.t) >= 1.0:
						this_step.t = self.t_motor.step()
						accumulation.t = accumulation.t - sign(steps.t)
					yield this_step

	def motor_step_distance(self):
		"""Return the step size of the motors."""
		return Polar(self.r_motor.step_distance, self.t_motor.step_distance)


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

	def spiral(self, radius_start, radius_end, pitch_count):
		"""A constant spiral."""
		radius_range = radius_end - radius_start
		radius_step = float(radius_range) / pitch_count

		turtle.up()
		start = Polar(radius_start, self.polar_error.t)
		self.setpos(start)

		turtle.down()
		for i in range(int(pitch_count)):
			rel_move = Polar(radius_step, math.tau)
			self.move(rel_move)
		turtle.up()

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

	def spiral(self, center, start_r, end_r, pitch_count):
		"""A constant spiral."""
		turtle.up()
		start = Polar(start_r, self.polar_error.t)
		self.setpos(start)

		turtle.down()
		for step in Spiral(start_r, end_r, pitch_count, self.polar_error.t):
			self.setpos(center + step.cartesian())
		turtle.up()


class PolarShape(object):
	
	def __init__(self, internal_position):
		self.internal_position = internal_position
		self.grid_size = PolarResolution(Millimeters(0.16), Degrees(0.05).radians())

	def step_absolute(self, new_position, minimum_step):
		"""Step in absolute polar positions."""
		delta = new_position - self.internal_position
		for rel_step in self.step_relative(delta, minimum_step):
			self.internal_position += rel_step
			yield self.internal_position

	def step_relative(self, delta, minimum_step):
		"""Step in relative polar positions."""
		steps_number = (delta / minimum_step).max_abs_value()
		delta_step = delta / float(steps_number)

		current = type(delta)(0.0, 0.0)
		for i in range(int(steps_number)):
			current += delta_step
			whole = Steps(current // minimum_step)
			current = current % minimum_step
			if whole.r == 0 and whole.t == 0:
				continue
			
			while abs(whole[0]) >= 1.0 or abs(whole[1]) >= 1.0:
				this_step = type(delta)(0.0, 0.0)
				if abs(whole[0]) >= 1.0:
					this_step[0] = minimum_step[0]
					whole[0] -= 1.0
				if abs(whole[1]) >= 1.0:
					this_step[1] = minimum_step[1]
					whole[1] -= 1.0
				yield this_step
		yield current


class Spiral(PolarShape):

	@staticmethod
	def archimedean(a, b, theta):
		"""Changing the parameter a will turn the spiral, while b controls the distance between successive turnings."""
		return a + b * theta
	
	@staticmethod
	def logarithmic(a, b, theta):
		"""Changing the parameter a will turn the spiral, while b controls the distance between successive turnings."""
		return a * math.e ** (b * theta)

	def __init__(self, radius_start, radius_end, turns, start_angle=None):

		super(Spiral, self).__init__(Polar(radius_start, start_angle))

		self.radius_start = radius_start
		self.radius_end = radius_end
		self.turns = turns

		radius_range = radius_end - radius_start
		self.radius_step = float(radius_range) / turns

		self.whole_step = Polar(self.radius_step, math.tau)

		if start_angle is None:
			start_angle = Radians(0.0)
		
		

		# steps_in_r = (self.radius_end - self.radius_start) / self.grid_size.r
		# steps_in_t = math.tau * self.turns / self.grid_size.t

	def __iter__(self):
		"""Iterable steps."""
		delta = Polar(self.radius_end - self.radius_start, math.tau * self.turns)
		for blah in self.step_absolute(delta, self.grid_size):
			yield blah

	def to_archimedean_parameters(self, radius_start, radius_end, turns):
		"""Build parameters for functions."""
		a = 0.0
		r = radius_end - radius_start
		theta = math.tau * turns
		
		b = (r - a) / theta
		return float(a), float(b), theta

	def to_logarithmic_parameters(self, radius_start, radius_end, turns):
		"""Build parameters for functions."""
		a = radius_start
		r = radius_end - radius_start
		theta = math.tau * turns
		
		# theta = 1 / b * math.log(r / a)
		b = math.log(r / a) / theta
		return a, b, theta

	def functor(self):
		func = Spiral.archimedean
		a, b, target_theta = self.to_archimedean_parameters(self.radius_start, self.radius_end, self.turns)

		func = Spiral.logarithmic
		a, b, target_theta = self.to_logarithmic_parameters(self.radius_start, self.radius_end, self.turns)
		
		while abs(self.internal_position.t) < abs(target_theta):
			delta = Polar(float(func(a, b, self.grid_size.t)), self.grid_size.t)
			for rel_step in self.step_relative(delta, self.grid_size):
				self.internal_position += rel_step
				yield self.internal_position
		

class Koru(PolarShape):

	def __init__(self):
		super(Koru, self).__init__(Polar(0.0, 0.0))

	def stem_func(self, b, theta):
		return float(b / math.sqrt(theta)) if theta >= 0.0 else 0.0

	def functor(self):
		stem_b = 50.0
		target_theta = Radians(math.tau * 1.5)
		
		theta_step = math.tau / 90.0  # self.grid_size.t
		while abs(self.internal_position.t) <= abs(target_theta):
			theta = self.internal_position.t + theta_step
			stem = self.stem_func(stem_b, theta)

			self.internal_position = Polar(stem, theta)
			yield self.internal_position

		stem_b = 30.0
		target_theta = Radians(0.0)
		self.internal_position.t -= math.tau
		while abs(self.internal_position.t) >= abs(target_theta):
			theta = self.internal_position.t - theta_step
			stem = self.stem_func(stem_b, theta)

			self.internal_position = Polar(stem, theta)
			yield self.internal_position
	
	def __iter__(self):
		"""Iterable steps."""
		for blah in self.functor():
			yield blah


class Transform(object):

	def __init__(self, shape, translation=None, rotation=None):
		self.shape = shape
		if translation is None:
			translation = Cartesian(0.0, 0.0)
		self.translation = translation
		if rotation is None:
			rotation = Radians(0.0)
		self.rotation = rotation

		self.pivot = Cartesian(0.0, 0.0)
	
	def __iter__(self):
		"""Iterable steps."""
		for blah in self.shape:
			blah.rotate(self.rotation)
			blah.translate(self.translation)
			yield blah


def lerp(a, b, weight):
	"""Linear interpilation between values."""
	return (weight * a) + ((1.0 - weight) * b)


def arc_center(p1, p2, radius, direction):
	"""Return arc center based on start and end points, radius and arc direction (2=CW, 3=CCW).
	
	radius can be negative (for arcs over 180 degrees)
	via https://math.stackexchange.com/a/482049
	"""
	angle = 0.0
	additional_angle = 0.0
	l1 = 0.0
	l2 = 0.0
	diff = 0.0
	allowed_error = 0.002
		
	t1 = None
	t2 = None
	
	if direction == CW:
		t1 = p2
		t2 = p1
	else:
		t1 = p1
		t2 = p2
	
	# find angle arc covers
	angle = calculate_angle(t1, t2)

	l1 = point_distance(t1, t2) / 2.0
	diff = l1 - abs(radius)

	if abs(radius) < l1 and diff > allowed_error:
		raise RuntimeError('Wrong radius.')
	
	l2 = math.sqrt(abs(radius ** 2 - l1 ** 2))

	if l1 == 0:
		additional_angle = math.pi / 2.0
	else:
		additional_angle = math.atan(l2 / l1)  # atan2???

	# Add or subtract from angle (depending of radius sign)
	if radius < 0:
		angle -= additional_angle
	else:
		angle += additional_angle

	# calculate center (from t1)
	x = float(t1.x + abs(radius) * math.cos(angle))
	y = float(t1.y + abs(radius) * math.sin(angle))
	return Cartesian(x, y)


def calculate_arc_length(center, p1, p2, radius, direction=None):
	"""Return the arc length and angle between the points."""
	d = (p2 - p1).length()
	theta = math.acos(1.0 - (d ** 2 / (2.0 * radius ** 2)))

	if direction == CCW:
		theta = math.tau - theta
	elif direction == CW:
		theta = theta
	else:
		theta = min(math.tau - theta, theta)

	arc_length = theta * radius

	return Millimeters(arc_length), Radians(theta)


def calculate_angle(p1, p2):
	"""Return angle of line between 2 points and X axis (according to quadrants)."""
	angle = 0.0

	if p1 == p2:  # same points
		return 0.0
	elif p1.x == p2.x:  # 90 or 270
		angle = math.pi / 2.0
		if p1.y > p2.y:
			angle += math.pi
	elif p1.y == p2.y:  # 0 or 180
		angle = 0.0
		if p1.x > p2.x:
			angle += math.pi
	else:
		angle = math.atan(abs((p2.y - p1.y) / (p2.x - p1.x)))  # 1. quadrant
		if p1.x > p2.x and p1.y < p2.y:  # 2. quadrant
			angle = math.pi - angle
		elif p1.x > p2.x and p1.y > p2.y:  # 3. quadrant
			angle += math.pi
		elif p1.x < p2.x and p1.y > p2.y:  # 4. quadrant
			angle = math.tau - angle
	return angle


def point_distance(p1, p2):
	"""Distance between points."""
	return math.sqrt((p2.x - p1.x) ** 2.0 + (p2.y - p1.y) ** 2.0)


def sign(value):
	"""Return -1 for negative values and 1 for positive."""
	return math.copysign(1, value)


def test_lines(plot):
	plot.draw_lines(
		Cartesian(0.0, 0.0),
		Cartesian(50.0, 0.0),
		Cartesian(50.0, 50.0),
		Cartesian(-150.0, -50.0),
		Cartesian(50.0, -80.0),
		Cartesian(-100.0, 90.0),
	)


def test_circles(plot):
	plot.draw_circle(Cartesian(0.0, 0.0), Millimeters(50.0))
	plot.draw_circle(Cartesian(100.0, 0.0), Millimeters(50.0))
	plot.draw_circle(Cartesian(-100.0, 0.0), Millimeters(50.0))
	plot.draw_circle(Cartesian(70.0, 70.0), Millimeters(50.0))


def test_arcs(plot):
	plot.draw_arc(Cartesian(0.0, 0.0), Millimeters(50.0), Radians(0.0), Radians(math.pi * 0.5))
	plot.draw_arc(Cartesian(0.0, 0.0), Millimeters(50.0), Radians(math.pi * 1.5), Radians(math.pi))

	plot.draw_arc(Cartesian(0.0, 0.0), Millimeters(60.0), Radians(math.pi * 0.5), Radians(math.pi))
	plot.draw_arc(Cartesian(0.0, 0.0), Millimeters(60.0), Radians(math.pi * 1.5), Radians(math.tau))

	plot.draw_arc(Cartesian(50.0, 50.0), Millimeters(60.0), Radians(math.pi * 0.5), Radians(math.pi))
	plot.draw_arc(Cartesian(-50.0, -50.0), Millimeters(60.0), Radians(math.pi * 1.5), Radians(math.tau))

	plot.draw_2point_arc(Cartesian(50.0, 0.0), Cartesian(0.0, 50.0), Millimeters(50.0), CW)
	plot.draw_2point_arc(Cartesian(50.0, 0.0), Cartesian(0.0, 50.0), Millimeters(50.0), CCW)
	plot.draw_2point_arc(Cartesian(50.0, 0.0), Cartesian(0.0, 50.0), Millimeters(100.0), CW)
	plot.draw_2point_arc(Cartesian(0.0, 50.0), Cartesian(50.0, 0.0), Millimeters(100.0), CW)

	plot.draw_3point_arc(Cartesian(50.0, 0.0), Cartesian(0.0, 50.0), Cartesian(-50.0, 0.0))
	plot.draw_3point_arc(Cartesian(0.0, 50.0), Cartesian(50.0, 0.0), Cartesian(-20.0, -70.0))


def test_polygons(plot):
	plot.draw_polygon(Cartesian(0.0, 0.0), Millimeters(50.0), 4)
	plot.draw_polygon(Cartesian(100.0, 0.0), Millimeters(50.0), 5)
	plot.draw_polygon(Cartesian(-100.0, 0.0), Millimeters(50.0), 6)
	plot.draw_polygon(Cartesian(70.0, 70.0), Millimeters(50.0), 3)


def test_polygon_arcs(plot):
	plot.draw_polygon_arc(Cartesian(90.0, -70.0), Millimeters(50.0), 3, Radians(math.pi), Radians(math.pi * 1.5))
	plot.draw_polygon_arc(Cartesian(-90.0, -70.0), Millimeters(50.0), 3, Radians(math.pi * 1.5), Radians(math.pi))

	plot.draw_polygon_arc(Cartesian(0.0, 0.0), Millimeters(50.0), 4, Radians(0.0), Radians(math.pi * 0.5))
	plot.draw_polygon_arc(Cartesian(100.0, 0.0), Millimeters(50.0), 5, Radians(0.0), Radians(math.pi * 0.5))
	plot.draw_polygon_arc(Cartesian(-100.0, 0.0), Millimeters(50.0), 6, Radians(0.0), Radians(math.pi * 0.5))
	plot.draw_polygon_arc(Cartesian(70.0, 70.0), Millimeters(50.0), 3, Radians(0.0), Radians(math.pi * 0.5))


# @profile(load_image=True)
def main():
	c = Controller()
	pol = PolarPlot(c)
	plot = CartesianPlot(pol)
	# plot.set_origin(Cartesian(-200, -200))

	# plot.move_fast(Cartesian(TABLE_RADIUS_MM, TABLE_RADIUS_MM))
	# plot.draw_circle(Cartesian(440.0, 0.0), Millimeters(50.0))

	# test_lines(plot)
	# test_circles(plot)
	# test_arcs(plot)
	# test_polygons(plot)
	# test_polygon_arcs(plot)

	# for i in range(1, 10):
	# 	pol.flower(i, 200)
	# 	time.sleep(1)
	# 	turtle.clear()

	# plot.spirograph()
	# pol.rhodonea(11, 31, 300)
	# pol.spiral(Millimeters(50.0), Millimeters(70.0), 3)
	pol.koru()



def exit():
	screen = Screen()
	screen.exitonclick()


if __name__ == '__main__':
	try:
		main()
		print 'done'
		exit()

	except (turtle_global.Terminator, tk.TclError):
		print 'closed'
