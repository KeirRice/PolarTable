#! /usr/bin/python
# -*- coding: utf-8 -*-

# Imports ####################################################################

import time

from utils import sign
from units import Milliseconds, Unit

from constants import FAST


# Class ######################################################################

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
		"""Step the motor."""
		self.set_direction(sign(steps))
		for i in range(int(abs(steps))):
			yield self.step()
			if not FAST:
				time.sleep(self.step_time)