import math

from units import PolarResolution, Millimeters, Degrees, Steps, Polar, Radians, Cartesian


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