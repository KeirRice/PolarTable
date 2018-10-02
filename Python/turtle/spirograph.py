from units import Cartesian, Polar, Millimeters, Radians, Inches
import uuid

import math
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


# 40 teeth per inch of diameter
diameter_per_tooth = Inches(1.0).millimeters() / 40.0
radius_per_tooth = diameter_per_tooth / 2.0


def hole_offset(teeth, hole_number):
	"""Convert spirograph hole numbers to mm offsets."""
	tooth_to_holes = dict([
		(24, 5),
		(30, 7),
		(32, 9),
		(36, 11),
		(40, 13),
		(42, 14),
		(45, 16),
		(48, 17),
		(50, 18),
		(52, 19),
		(56, 21),
		(60, 23),
		(63, 25),
		(64, 25),
		(72, 29),
		(75, 31),
		(80, 33),
		(84, 35)
	])
	step_per_hole = (radius_per_tooth * teeth - 2.5) / float(tooth_to_holes[teeth])
	return 1.0 + (step_per_hole * hole_number)


class Cog(object):
	"""A single cog."""

	def __init__(self, radius, speed=1.0):
		"""Init.

		radius of the cog
		speed multiplies the turning rate relative to the parent.
		"""
		self.radius = Millimeters(radius)
		self.offset = None

		self.children = list()
		self.track = Polar(0.0, 0.0)
		
		self.theta = Radians(0.0)
		self.speed_boost = speed
		self.base_speed = 1.0
		self.center = Cartesian(0.0, 0.0)
		self.direction = 1.0

		self.parent_radius = None

		self.id = uuid.uuid4()

	def start(self, start_angle):
		"""Push the first angle through the stack."""
		self.set_theta(start_angle)

	def steps(self, edges_per_mm=0.1):
		"""Run the cogs."""
		orbits = self.repeats()
		circumference = float(self.radius) * math.tau
		edge_count = circumference * float(edges_per_mm) * orbits
		steps = int(math.ceil(edge_count))
		
		delta_theta = Radians(orbits * math.tau / steps)
		theta = Radians(0.0)
		for i in range(steps + 1):
			theta += delta_theta
			self.set_theta(theta)
			yield i

	def add_child(self, child):
		"""Attach a child to this cog."""
		if not isinstance(child, type(self)):
			raise RuntimeError('Not a cog.')

		self.children.append(child)

		# Cogs flip the direction of travel at each connection.
		child.direction = self.direction * -1.0
		child.base_speed = float(self.radius) / child.radius * child.direction

		# Default track is so the radius will just touch
		child.set_track(self.radius - child.radius, 0.0)
		# child.parent_radius(self.radius)
		return self
	
	def remove_child(self, child):
		"""Remove a cog."""
		index = self.children.index(child)
		if index != -1:
			del self.children[index]
		return self

	def set_track(self, radius, angle):
		"""Save the track."""
		self.track = Polar(radius, angle)

	def repeat_after_mm(self):
		"""Recurse to find how far we need to go to repeat."""
		base = self.radius
		for child in self.children:
			base = math.lcm(base, child.repeat_after_mm())
		return base
		
	def repeats(self):
		"""Return the number of revolutions till we return to the start."""
		repeat_after_orbits = float(self.repeat_after_mm()) / self.radius
		return repeat_after_orbits

	def __add__(self, other):
		"""Attach."""
		if isinstance(other, type(self)):
			return self.add_child(other)
		return NotImplemented

	def __sub__(self, other):
		"""Remove."""
		if isinstance(other, type(self)):
			return self.remove_child(other)
		return NotImplemented

	def __eq__(self, other):
		"""Equality."""
		if isinstance(other, type(self)):
			return self.id == other.id
		return NotImplemented

	def __str__(self):
		"""String."""
		return 'Cog({}, {})'.format(self.radius, self.speed_boost)

	def set_theta(self, theta):
		"""Update the angle."""
		self.theta = theta * self.base_speed * self.speed_boost
		# Display.debug_circle(self.center, self.radius)

		for child in self.children:
			child.track.t = self.theta
			
			child.center = self.center + child.track.cartesian()
			child.set_theta(self.theta)


class ToothedGear(Cog):
	"""Build gear base on spirograph tooth count."""

	def __init__(self, teeth):
		"""Init."""
		self.teeth = teeth
		radius = teeth * radius_per_tooth
		super(ToothedGear, self).__init__(radius, speed=1.0)
		

class Pen(Cog):
	"""docstring for Pen."""

	def __init__(self, offset, color):
		"""Init."""
		super(Pen, self).__init__(1.0)
		self.offset = offset
		self.color = color

	def __str__(self):
		"""String."""
		return 'Pen({}, {})'.format(self.offset, self.color)

	def set_track(self, radius, angle):
		"""The pen's track can be anywhere."""
		self.track = Polar(self.offset, angle)

	def set_theta(self, theta):
		"""Update and draw."""
		super(Pen, self).set_theta(theta)
		# self.plot.setpos(self.center)
		# Display.debug_point(self.center, self.color)
	
	def position(self):
		"""Current pen position."""
		return self.center

	def add_child(self, child):
		"""Disable."""
		raise RuntimeError('Pens can not have children.')
		
	def remove_child(self, child):
		"""Disable."""
		raise RuntimeError('Pens can not have children.')


# a = Cog(300)
# b = Cog(110)
# # c = Cog(self, 10, 100.0, 5)
# # d = Cog(self, 50)

# a + b
# b + Pen(self, 50, 'red')
# b + Pen(self, 60, 'blue')
# # b + c
# # a + d

# theta = Radians(0.0)
# for i in range(9999):
# 	theta += Radians(0.01)
# 	a.set_theta(theta)
