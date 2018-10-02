#! /usr/bin/python
# -*- coding: utf-8 -*-

# Imports ####################################################################

import math

from units import Cartesian, Millimeters, Radians

from constants import CW, CCW

# Functions ##################################################################

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
