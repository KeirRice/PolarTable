#! /usr/bin/python
# -*- coding: utf-8 -*-

import math
math.tau = math.pi * 2.0
import numbers

PIXELS_PER_MM = 10.0  # scale 10 units per mm
FAST = False


def convert_units(func):
	"""Decorator to try and align unit types before calculation."""
	def wrapper(self, other):
		if not isinstance(other, type(self)) and isinstance(other, Unit):
			converted = self.convert(other)
			if converted is not NotImplemented:
				other = converted
		return func(self, other)

	return wrapper


class Unit(object):
	"""Units base."""
	
	def __init__(self, value, suffix=''):
		"""Init."""
		# If we are passed in an existing unit unpack it.
		if isinstance(value, Unit):
			if suffix == value.suffix:
				value = value.value
			elif value.suffix == '' or suffix == '':
				value = value.value
				suffix = ''
			else:
				print 'suffix = "{}", value = "{}"'.format(suffix, value)
				raise RuntimeError('Converion fail.')

		if not isinstance(value, numbers.Number):
			raise ValueError('Units only support numbers. Got {}'.format(repr(value)))
		self.value = value
		self.suffix = suffix

		self.is_boundry = False

	def convert(self, other):
		"""Convert other to try and match ourselfs."""
		if isinstance(other, type(self)):
			return other
		if hasattr(other, 'conversions'):
			if type(self) in other.conversions:
				return other.conversions[type(self)]()
		return NotImplemented

	def units(self):
		"""Strip the units."""
		return Unit(self.value, '')
		
	def __str__(self):
		"""Str."""
		value = '{:0.3f}'.format(self.value)
		if '.' in value:
			value = value.rstrip('0')
			if value[-1] == '.':
				value += '0'
		return '{}{}'.format(value, self.suffix)
	
	def __repr__(self):
		"""Repr."""
		return '{}({})'.format(self.__class__.__name__, str(self))

	def __eq__(self, other):
		"""Equality."""
		if isinstance(other, type(self)):
			return self.value == other.value
		return self.value == other

	def __ne__(self, other):
		"""Equality."""
		if isinstance(other, type(self)):
			return self.value != other.value
		return self.value != other

	def __le__(self, other):
		"""Less equality."""
		if isinstance(other, type(self)):
			return self.value <= other.value
		return self.value <= other

	def __lt__(self, other):
		"""Less than."""
		if isinstance(other, type(self)):
			return self.value < other.value
		return self.value < other

	def __ge__(self, other):
		"""Less equality."""
		if isinstance(other, type(self)):
			return self.value >= other.value
		return self.value >= other

	def __gt__(self, other):
		"""Less than."""
		if isinstance(other, type(self)):
			return self.value > other.value
		return self.value > other

	@convert_units
	def __add__(self, other):
		"""Add."""
		if isinstance(other, type(self)):
			return type(self)(self.value + other.value)
		elif isinstance(other, numbers.Number):
			return type(self)(self.value + other)
		elif isinstance(other, Unit):
			return Unit(self.value + other.value)
		return NotImplemented

	__radd__ = __add__

	@convert_units
	def __sub__(self, other):
		"""Subtract."""
		if isinstance(other, type(self)):
			return type(self)(self.value - other.value)
		elif isinstance(other, numbers.Number):
			return type(self)(self.value - other)
		elif isinstance(other, Unit):
			return Unit(self.value - other.value)
		return NotImplemented

	def __rsub__(self, other):
		"""Subtract."""
		if isinstance(other, numbers.Number):
			return type(self)(other - self.value)
		return NotImplemented

	@convert_units
	def __div__(self, other):
		"""Division."""
		if isinstance(other, type(self)):
			return type(self)(self.value / other.value)
		elif isinstance(other, numbers.Number):
			return type(self)(self.value / other)
		elif isinstance(other, Unit):
			return Unit(self.value / other.value)
		return NotImplemented

	def __rdiv__(self, other):
		"""Division."""
		if isinstance(other, numbers.Number):
			return type(other)(other / self.value)
		return NotImplemented

	@convert_units
	def __floordiv__(self, other):
		"""Floor division."""
		if isinstance(other, type(self)):
			return type(self)(math.floor(self.value // other.value))
		elif isinstance(other, numbers.Number):
			return type(self)(math.floor(self.value // other))
		return NotImplemented

	@convert_units
	def __mul__(self, other):
		"""Multiplication."""
		if isinstance(other, type(self)):
			return type(self)(self.value * other.value)
		elif isinstance(other, numbers.Number):
			return type(self)(self.value * other)
		elif isinstance(other, Unit):
			return Unit(self.value * other.value)
		return NotImplemented

	__rmul__ = __mul__

	def __mod__(self, other):
		"""Modulo."""
		if isinstance(other, type(self)):
			return type(self)(self.value % other.value)
		elif isinstance(other, numbers.Number):
			return type(self)(self.value % other)
		return NotImplemented

	__rmod__ = __mod__

	def __pow__(self, other):
		"""Power."""
		if isinstance(other, type(self)):
			return type(self)(self.value ** other.value)
		elif isinstance(other, numbers.Number):
			return type(self)(self.value ** other)
		return NotImplemented

	def __rpow__(self, other):
		"""Power."""
		if isinstance(other, numbers.Number):
			return type(self)(other ** self.value)
		return NotImplemented

	def __iadd__(self, other):
		"""Add in place."""
		if isinstance(other, type(self)):
			self.value = self.value + other.value
			return self
		return NotImplemented

	def __isub__(self, other):
		"""Subtract in place."""
		if isinstance(other, type(self)):
			self.value = self.value - other.value
			return self
		return NotImplemented

	def __idiv__(self, other):
		"""Division in place."""
		if isinstance(other, type(self)):
			self.value = self.value / other.value
			return self
		elif isinstance(other, numbers.Number):
			self.value = self.value / other
			return self
		return NotImplemented

	def __imul__(self, other):
		"""Division."""
		if isinstance(other, type(self)):
			self.value = self.value * other.value
			return self
		elif isinstance(other, numbers.Number):
			self.value = self.value * other
			return self
		return NotImplemented

	def __abs__(self):
		"""Absolute."""
		return type(self)(abs(self.value))

	def __neg__(self):
		"""Negate."""
		return type(self)(-self.value)

	def __pos__(self):
		"""Positive."""
		return type(self)(+self.value)

	def __int__(self):
		"""To int."""
		return int(self.value)

	def __long__(self):
		"""To long."""
		return long(self.value)

	def __float__(self):
		"""To float."""
		return float(self.value)


class Distance(Unit):
	"""Distances base."""

	PIXELS_PER_MM = 1.0
	MM_PER_INCH = 25.4

	@classmethod
	def set_conversion_factor(cls, pixels, mm):
		"""Dynamically set the pixel conversion factor."""
		cls.PIXELS_PER_MM = float(pixels) / mm


class Millimeters(Distance):
	"""Distances base."""

	def __init__(self, value):
		"""Init."""
		super(Millimeters, self).__init__(value, 'mm')
		self.conversions = {
			Pixels: self.pixels,
			Inches: self.inches,
		}

	def pixels(self):
		"""Convert to pixels."""
		return Pixels(self.value * self.PIXELS_PER_MM)

	def inches(self):
		"""Convert to pixels."""
		return Inches(self.value / self.MM_PER_INCH)


class Inches(Distance):
	"""Distances base."""

	def __init__(self, value):
		"""Init."""
		super(Inches, self).__init__(value, 'inch')
		self.conversions = {
			Pixels: self.pixels,
			Millimeters: self.millimeters,
		}

	def pixels(self):
		"""Convert to pixels."""
		return self.millimeters().pixels()

	def millimeters(self):
		"""Convert to pixels."""
		return Millimeters(self.value * self.MM_PER_INCH)


class Pixels(Distance):
	"""Distances base."""

	def __init__(self, value):
		"""Init."""
		super(Pixels, self).__init__(value, 'px')
		self.conversions = {
			Millimeters: self.millimeters,
			Inches: self.inches,
		}

	def millimeters(self):
		"""Convert to millimeters."""
		return Millimeters(float(self.value) / self.PIXELS_PER_MM)

	def inches(self):
		"""Convert to inches."""
		return self.millimeters().inches()


class Angle(Unit):
	"""Angle baseclass."""

	pass


class Radians(Angle):
	"""Radianss."""

	def __init__(self, value):
		"""Init."""
		super(Radians, self).__init__(value, 'rad')

	def __eq__(self, other):
		"""Equality."""
		if isinstance(other, Degrees):
			return self.value == other.radians()
		return super(Radians, self).__eq__(other)

	def degrees(self):
		"""Convert to degrees."""
		return Degrees(math.degrees(self.value))


class Degrees(Angle):
	"""Degrees."""

	def __init__(self, value):
		"""Init."""
		super(Degrees, self).__init__(value, 'deg')

	def __eq__(self, other):
		"""Equality."""
		if isinstance(other, Radians):
			return self.value == other.degrees()
		return super(Radians, self).__eq__(other)

	def radians(self):
		"""Convert to radians."""
		return Radians(math.radians(self.value))


class Time(Unit):
	"""Base class for time units."""

	pass


class Seconds(Time):
	"""Milli Seconds."""

	def __init__(self, value):
		"""Init."""
		super(Seconds, self).__init__(value, 's')
	
	def milliseconds(self):
		"""Convert to seconds."""
		return Seconds(self.value * 1000.0)


class Milliseconds(Time):
	"""Milli Seconds."""
	
	def __init__(self, value):
		"""Init."""
		super(Milliseconds, self).__init__(value, 'ms')

	def seconds(self):
		"""Convert to seconds."""
		return Seconds(self.value / 1000.0)
		

class Coordinates(object):
	"""Base for working with coordinates."""
	
	def __init__(self, labels, *values):
		"""Init.
		
		Looks weird because we are overriding __getattr__
		"""
		super(Coordinates, self).__setattr__('labels', labels)
		super(Coordinates, self).__setattr__('storage', list(values))
		super(Coordinates, self).__setattr__('lookup', dict([(name, i) for i, name in enumerate(self.labels)]))

	def copy(self):
		"""Create a copy of the coordinate."""
		return type(self)(*self.storage)

	def units(self):
		"""Convert all values to units."""
		return type(self)(*[v.units() for v in self])

	def pixels(self):
		"""Convert all values to units."""
		return type(self)(*[v.pixels() for v in self])

	def __getattr__(self, name):
		"""Enable . notation access to our values."""
		if name in self.__dict__['labels']:
			return self.__dict__['storage'][self.__dict__['lookup'][name]]
		if name in self.__dict__:
			return self.__dict__[name]
		raise AttributeError

	def __setattr__(self, name, value):
		"""Enable . notation setting of our values."""
		if name in self.labels:
			self.storage[self.lookup[name]] = value
		if name in self.__dict__:
			self.__dict__[name] = value

	def __getitem__(self, key):
		"""Enable indexed access to values."""
		return self.storage[key]

	def __setitem__(self, key, value):
		"""Enable indexed setting of values."""
		self.storage[key] = value

	def __eq__(self, other):
		"""Equality."""
		if isinstance(other, Coordinates):
			return self.storage == other.storage
		return tuple(self.storage) == other

	def __iter__(self):
		"""Iteration of values."""
		for value in self.storage:
			yield value

	def __str__(self):
		"""String showing type and values."""
		values = ','.join(['{}={}'.format(l, v) for v, l in zip(self, self.labels)])
		return '{}({})'.format(self.__class__.__name__, values)

	def __add__(self, other):
		"""Add."""
		if isinstance(other, type(self)):
			return type(self)(*[s + o for s, o in zip(self, other)])
		return NotImplemented

	def __sub__(self, other):
		"""Subtract."""
		if isinstance(other, type(self)):
			return type(self)(*[s - o for s, o in zip(self, other)])
		return NotImplemented

	def __div__(self, other):
		"""Division."""
		if isinstance(other, type(self)):
			return type(self)(*[float(s) / float(o) for s, o in zip(self, other)])
		elif isinstance(other, Unit):
			return type(self)(*[float(s) / other for s in self])
		elif isinstance(other, numbers.Number):
			return type(self)(*[float(s) / other for s in self])
		return NotImplemented

	def __mul__(self, other):
		"""Division."""
		if isinstance(other, type(self)):
			return type(self)(*[float(s) * float(o) for s, o in zip(self, other)])
		if isinstance(other, Steps):
			return type(self)(*[float(s) * float(o) for s, o in zip(self, other)])
		elif isinstance(other, Unit):
			return type(self)(*[float(s) * other for s in self])
		elif isinstance(other, numbers.Number):
			return type(self)(*[float(s) * other for s in self])
		return NotImplemented

	def __rdiv__(self, other):
		"""Division."""
		if isinstance(other, numbers.Number):
			return type(self)(*[other / float(s) for s in self])
		return NotImplemented

	def __iadd__(self, other):
		"""Add in place."""
		if isinstance(other, type(self)):
			self.storage = [s + o for s, o in zip(self, other)]
			return self
		return NotImplemented

	def __isub__(self, other):
		"""Subtract in place."""
		if isinstance(other, type(self)):
			self.storage = [s - o for s, o in zip(self, other)]
			return self
		return NotImplemented

	def __idiv__(self, other):
		"""Division in place."""
		if isinstance(other, type(self)):
			self.storage = [s / o for s, o in zip(self, other)]
			return self
		elif isinstance(other, Unit):
			self.storage = [s / other for s in self]
			return self
		elif isinstance(other, numbers.Number):
			self.storage = [s / other for s in self]
			return self
		return NotImplemented

	def __mod__(self, other):
		"""Modulas."""
		if isinstance(other, type(self)):
			return type(self)(*[s % o for s, o in zip(self, other)])
		return type(self)(*[s % other for s in self])
		
	def __floordiv__(self, other):
		"""Floor div."""
		if isinstance(other, type(self)):
			return type(self)(*[s // o for s, o in zip(self, other)])
		if isinstance(other, Coordinates):
			return type(self)(*[s // o for s, o in zip(self, other)])
		return type(self)(*[s // other for s in self])

	def angle(self, other):
		"""Angle between self and other."""
		return Radians(math.acos(self.dot(other)))

	def dot(self, other):
		"""Dot product."""
		return sum([i * j for (i, j) in zip(self.normalised(), other.normalised())])

	def length(self):
		"""Length."""
		return math.sqrt(sum([n**2 for n in self]))

	def normalised(self):
		"""Return a normalised vector of our selfes."""
		length = self.length()
		try:
			return [n / length for n in self]
		except ZeroDivisionError:
			print self
			raise

	def round(self):
		"""Round to whole numbers."""
		return type(self)(*[int(round(v)) for v in self])

	def trunc(self, factor):
		"""Round to whole numbers."""
		return type(self)(*[float(int(v * factor)) / factor for v in self])

	def max_abs_value(self):
		"""Return the maximum absolute value int the coordinate."""
		return float(max(map(abs, self)))

	def modf(self):
		"""Split the coordinate into two new coordinates. The Fraction and the Whole."""
		wholes = list()
		fracs = list()
		for value in self:
			f, w = math.modf(value)
			fracs.append(f)
			wholes.append(w)

		return (type(self)(*fracs), type(self)(*wholes))


class Steps(Coordinates):
	"""Step coordinates."""
	
	def __init__(self, *args, **kwargs):
		"""Init."""
		if len(args) == 2:
			r, t = args
		else:
			r = args[0][0]
			t = args[0][1]

		if kwargs.get('polar', True):
			labels = ['r', 't']
		else:
			labels = ['x', 'y']
			if kwargs.get('cartesian', True):
				labels = ['x', 'y']
			else:
				labels = ['r', 't']

		super(Steps, self).__init__(labels, Unit(r), Unit(t))


class Cartesian(Coordinates):
	"""Cartesian coordinates."""
	
	def __init__(self, x, y):
		"""Init."""
		if not isinstance(x, Unit):
			x = Millimeters(x)
		if not isinstance(y, Unit):
			y = Millimeters(y)
		super(Cartesian, self).__init__(['x', 'y'], x, y)

	def rotate(self, theta, pivot=None):
		if pivot:
			self -= pivot
		x = self.x * math.cos(theta) - self.y * math.sin(theta)
		y = self.x * math.sin(theta) + self.y * math.cos(theta)
		self.x = x
		self.y = y
		if pivot:
			self += pivot

	def translate(self, offset):
		print self, 'offset', offset
		self.x += offset.x
		self.y += offset.y

	def polar(self):
		"""Convert to Polar."""
		x, y = float(self.x), float(self.y)
		r = Millimeters(math.sqrt(pow(x, 2) + pow(y, 2)))

		# atan has a discontinuity at pi.
		t = Radians((math.atan2(y, x) + math.tau) % math.tau)
		return Polar(r, t)

	def pixels(self):
		"""Convert to pixels."""
		return DisplayPosition(*[v.pixels() for v in self])


class CartesianResolution(Cartesian):
	"""Grid size for Cartesian"""

	pass


class DisplayResolution(Coordinates):
	"""Grid size for Cartesian"""

	def __init__(self, x, y):
		"""Init."""
		if not isinstance(x, Unit):
			x = Pixels(x)
		if not isinstance(y, Unit):
			y = Pixels(y)
		super(DisplayResolution, self).__init__(['x', 'y'], x, y)


class DisplayPosition(DisplayResolution):

	pass


class Polar(Coordinates):
	"""Polar coordinates."""
	
	def __init__(self, r, t):
		"""Init."""
		if not isinstance(r, Unit):
			r = Millimeters(r)
		if not isinstance(t, Unit):
			t = Radians(t)
		super(Polar, self).__init__(['r', 't'], r, t)

	def rotate(self, theta, pivot=None):
		if pivot is None or pivot == Cartesian(0.0, 0.0) or pivot == Polar(0.0, 0.0):
			self.t += theta
		else:
			cartesian = self.cartesian()
			cartesian.rotate(theta, pivot)
			polar = cartesian.polar()
			self.r = polar.r
			self.t = polar.t

	def translate(self, offset):
		cartesian = self.cartesian()
		cartesian.translate(offset)
		polar = cartesian.polar()
		self.r = polar.r
		self.t = polar.t

	def cartesian(self):
		"""Cartesian to Polar."""
		r = float(self.r)
		t = float(self.t)
		return Cartesian(r * math.cos(t), r * math.sin(t))


class PolarResolution(Polar):
	"""Grid size for Polar"""

	pass


class TkSpace(Coordinates):
	"""Pixels units for coordinates in tkinter widget space."""

	def __init__(self, x, y):
		"""Init."""
		if not isinstance(x, Pixels):
			x = Millimeters(x)
		if not isinstance(y, Pixels):
			y = Millimeters(y)
		super(TkSpace, self).__init__(['x', 'y'], x, y)
