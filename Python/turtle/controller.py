#! /usr/bin/python
# -*- coding: utf-8 -*-

# Imports ####################################################################

from display import Display
from motor import Motor
from constants import TABLE_RADIUS_MM, FAST_SKIP_MOTORS
from utils import sign
from units import Millimeters, Degrees, PolarResolution, Steps, Polar


# Class ######################################################################

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
		return u'Controller(<{0}>, <{1}>)'.format(self.t_motor, self.r_motor)

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