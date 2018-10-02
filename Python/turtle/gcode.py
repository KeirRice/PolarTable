#! /usr/bin/python
# -*- coding: utf-8 -*-

import re

import controller
import plots.cartesian
import plots.polar
import units
import time
import turtle_table

CW, CCW = 'Clockwise', 'Counter-clockwise'

"""
G-Codes
G0 - Rapid Motion
G1 - Coordinated Motion
G2 - Arc - Clockwise
G3 - Arc - Counter Clockwise
G4 - Dwell
G20 - Inches as units
G21 - Millimeters as units
G28 - Go Home
G30 - Go Home via Intermediate Point
G90 - Absolute Positioning
G91 - Incremental Positioning
G92 - Set current as home

F	Defines feed rate
N	Line (block) number in program; System parameter number to change using G10
O	Program name
P	Serves as parameter address for various G and M codes
R	Defines size of arc radius, or defines retract height in milling canned cycles
U	Incremental axis corresponding to X axis (typically only lathe group A controls) Also defines dwell time on some machines (instead of "P" or "X").
V	Incremental axis corresponding to Y axis
W	Incremental axis corresponding to Z axis (typically only lathe group A controls)
X	Absolute or incremental position of X axis. Also defines dwell time on some machines (instead of "P" or "U").
Y	Absolute or incremental position of Y axis
Z	Absolute or incremental position of Z axis


M117 Display message
"""

test_one = """
G17 G20 G90 G94 G54
G0 Z0.25
X-50. Y0.
Z0.1
G01 Z0. F5.
G02 X0. Y50. I50. J0. F2.5
X50. Y0. I0. J-50.
X0. Y-50. I-50. J0.
X-50. Y0. I0. J50.
G01 Z0.1 F5.
G00 X0. Y0. Z0.25
"""

test_two = """
G0 Y10 Z-5
G1 Z-10
G1 Y20
G02 X10 Y30 R10
G1 X30
G2 X40 Y20 R10
G1 Y10
G2 X30 Y0 R10
G1 X10
G2 X0 Y10 Z-15 R10 (yeah spiral !)
G3 X-10 Y20 R-10 (yeah, long arc !)
G3 X0 Y10 I10 (center)
G91 G1 X10 Z10
G3 Y10 R5 Z3 (circle in incremental)
Y10 R5 Z3 (again, testing modal state)
G20 G0 X1 (one inch to the right)
G3 X-1 R1 (radius in inches)
G3 X1 Z0.3 I0.5 J0.5 (I,J in inches)
G21 (back to mm)
G80 X10 (do nothing)
G90
G0 X30 Y30 Z30
"""

test_three = """
G90
G1 Z-0.50
G1 Y2.00
G2 X1.00 Y3.00 I1.00 J0.00
G1 X4.00
G2 X5.00 Y2.00 I0.00 J-1.00
G1 Y0.00
G2 X4.00 Y-1.00 I-1.00 J0.00
G1 X1.00 Y-1.00
G2 X0.00 Y0.00 I0.00 J1.00
G0 Z0.00
"""


class FeatureNotSupported(RuntimeError):
	pass


class Machine(object):
	"""Maps from gcode to CartesianPlot."""

	class Word(object):
		"""Gcode words."""

		def __init__(self, letter, value, message=None):
			"""Init."""
			self.letter = letter
			try:
				self.value = float(value) if '.' in value else int(value)
			except ValueError:
				self.value = value
			self.message = message

		def __str__(self):
			"""String."""
			return "Word('{}', {})".format(self.letter, self.value)

	class Command(object):
		"""Command."""

		def __init__(self, controller, state, line_number, raw_text):
			"""Init."""
			self.controller = controller
			self.line_number = line_number
			self.raw_text = raw_text

			self.state = state
			self.message = None

			self._xyz = [None, None, None]
			self.radius = None
			self._ijk = [None, None, None]
			self.dwell_time = None
			self.feedrate = None

		@property
		def pos(self):
			"""Position of the controller.

			It's important that this is up to date at the time of execution.
			Don't pre cache it early.
			"""
			return self.controller.pos()

		@property
		def units(self):
			"""Return active units."""
			if self.state['units'] == Machine.UNITS_INCHES:
				return units.Inches
			return units.Millimeters
		
		@property
		def scale(self):
			"""Return the scale modifier."""
			return self.state['scale']

		@property
		def is_absolute(self):
			"""True if we are in absolute positioning mode."""
			return self.state['postioning'] == Machine.POSTIONING_ABSOLUTE

		@property
		def is_relative(self):
			"""True if we are in relative positioning mode."""
			return self.state['postioning'] == Machine.POSTIONING_RELATIVE

		@property
		def xy(self):
			"""Get xy as a cartesian pair."""
			point = self._xyz[:2]
			if len(point) == 0:
				print 'No points found.'
				return
			if point == [None, None]:
				print 'No xy point value {}.'.format(point)
				return

			# Overlay the set values on top of our current position.
			if self.is_relative:
				base = units.Cartesian(self.units(0.0), self.units(0.0))
			else:  # self.is_absolute
				base = self.pos
			for i, _ in enumerate(point):
				if point[i] is not None:
					base[i] = point[i] * self.scale
			return base

		@property
		def ij(self):
			"""Get ij as a cartesian pair."""
			point = self._ijk[:2]
			if len(point) == 0:
				print 'No points found.'
				return
			if point == [None, None]:
				print 'No ij point value {}.'.format(point)
				return

			# Overlay the set values ontop of our current position.
			base = units.Cartesian(self.units(0.0), self.units(0.0))
			for i, _ in enumerate(point):
				if point[i] is not None:
					base[i] = point[i] * self.scale
			return base

		def set_motion(self, value):
			"""Set the motion type to one of the Machine.MOTION_* constants."""
			self.state['motion_mode'] = value
		
		def set_message(self, value):
			"""Set a message to display to the user."""
			self.message = value

		def set_units(self, value):
			"""Working units."""
			self.state['units'] = value

		def set_postioning(self, value):
			"""Set the postioning type to one of the Machine.POSTIONING_* constants."""
			self.state['postioning'] = value

		def set_feedrate(self, value):
			"""Set how fast we should move."""
			self.state['feedrate'] = self.feedrate = self.units(value) * self.scale

		def set_radius(self, value):
			"""Set radius for arcs."""
			self.radius = self.units(value) * self.scale

		def set_arc_position(self, index, value):
			"""Set conter point for arc."""
			self._ijk[index] = self.units(value)

		def set_position(self, index, value):
			"""Position."""
			self._xyz[index] = self.units(value)

		def set_dwell(self, value):
			"""Dwell (sleep)."""
			self.dwell_time = units.Milliseconds(value)

		def execute(self):
			"""Run the commands."""
			if self.message:
				print self.message
			self.execute_feedrate()
			self.execute_motion()

		def execute_feedrate(self):
			"""Apply feedrate to controller."""
			if self.feedrate is not None:
				self.controller.set_feedrate(self.feedrate)

		def execute_motion(self):
			"""Apply the motion to the controller.

			This should get executed just in time.
			"""
			controller = self.controller
			if self.state['motion_mode'] in (Machine.MOTION_RAPID, Machine.MOTION_COORDINATED):
				point = self.xy
				if point:
					position = units.Cartesian(*point)

					command_lookup = {
						(Machine.POSTIONING_ABSOLUTE, Machine.MOTION_RAPID): controller.setpos_fast,
						(Machine.POSTIONING_ABSOLUTE, Machine.MOTION_COORDINATED): controller.setpos,
						(Machine.POSTIONING_RELATIVE, Machine.MOTION_RAPID): controller.move_fast,
						(Machine.POSTIONING_RELATIVE, Machine.MOTION_COORDINATED): controller.move,
					}
					command_lookup[(self.state['postioning'], self.state['motion_mode'])](position)
					
			elif self.state['motion_mode'] in (Machine.MOTION_ARC_CW, Machine.MOTION_ARC_CCW):
				position = self.xy
				if position:
					if self.state['postioning'] == Machine.POSTIONING_RELATIVE:
						position = controller.to_absolute(position)

					direction = CW if self.state['motion_mode'] == Machine.MOTION_ARC_CW else CCW
					if self.radius is not None:
						controller.draw_2point_arc(None, position, self.radius, direction)
					else:
						point = self.ij
						if point:
							center = units.Cartesian(*point)
							controller.draw_3point_arc(None, position, center, direction)

			elif self.state['motion_mode'] == Machine.MOTION_DWELL:
				time.sleep(self.dwell_time)

			elif self.state['motion_mode'] is None:
				pass

			else:
				raise FeatureNotSupported('Unsupported motion state.')

	UNITS_INCHES, UNITS_MM = range(2)
	POSTIONING_ABSOLUTE, POSTIONING_RELATIVE = range(2)
	MOTION_RAPID, MOTION_COORDINATED, MOTION_ARC_CW, MOTION_ARC_CCW, MOTION_DWELL = range(5)
	PLANE_XY, PLANE_ZX, PLANE_YZ = range(3)

	def __init__(self, plot, scale=1.0):
		"""Init."""
		self.plot = plot
		self.state = {
			'motion_mode': None,
			'units': self.UNITS_MM,
			'postioning': self.POSTIONING_ABSOLUTE,
			'plane': self.PLANE_XY,
			'feedrate': None,
			'home': units.Cartesian(0.0, 0.0),
			'scale': scale
		}

		self.line_number = 0
		# TODO: Buffer the lines to process
		# TODO: Buffer the commands

	def get_state(self):
		"""Get the current global state."""
		return self.state

	def set_state(self, state):
		"""Update the current global state."""
		self.state.update(state)

	def parse_line(self, line):
		"""Parse a g-code line."""
		
		# Strip comments
		line = re.sub(r'\(.*\)', ' ', line)
		line = re.sub(r';.*$', '', line)

		# Remove double spaces
		line = re.sub(r'\s+', ' ', line)
		
		line_words = iter(line.split(' '))
		words = list()
		for word in line_words:
			word = word.strip()
			if not word:
				continue

			letter = word[0]  # first letter
			code = word[1:]  # rest of string

			if letter == 'M':
				if code == '117':
					message = line.split('M117', 1)[-1]
					words.append(Machine.Word(letter, code, message))
					break
				if code == '72':
					# Look ahead for the P parameter
					ptext = line_words.next()
					# Use it to extract the following message
					message = line.split(ptext, 1)[-1]
					# Build the words
					words.append(Machine.Word(letter, code, message))
					words.append(Machine.Word(ptext[0], ptext[1:]))
					break

			words.append(Machine.Word(letter, code))

		return words

	def parse_lines(self, lines):
		"""Process a list of lines or split the string on newlines."""
		if isinstance(lines, basestring):
			lines = lines.split('\n')

		for line in lines:
			self.line_number += 1
			
			words = self.parse_line(line)
			if not words:
				continue

			command = Machine.Command(self.plot, self.state, self.line_number, line)
			self.process_line(command, words)
			self.set_state(command.state)
			command.execute()

	def parse_file(self, fh):
		"""Parse line from files."""
		for line in fh.readlines():
			line = line.strip()
			self.parse_lines([line])

	def process_line(self, command, words):
		"""Process the words into the command."""
		for word in words:
			if word.letter == 'G':
				if word.value == 0:
					command.set_motion(self.MOTION_RAPID)
				elif word.value == 1:
					command.set_motion(self.MOTION_COORDINATED)
				elif word.value == 2:
					command.set_motion(self.MOTION_ARC_CW)
				elif word.value == 3:
					command.set_motion(self.MOTION_ARC_CCW)
				elif word.value == 4:
					command.set_motion(self.MOTION_DWELL)

				if word.value == 17:
					command.state['plane'] = self.PLANE_XY
				elif word.value == 18:
					raise FeatureNotSupported('G18	ZX plane selection')
					# self.plane = self.PLANE_ZX
				elif word.value == 19:
					raise FeatureNotSupported('G19	YZ plane selection')
					# self.plane = self.PLANE_YZ

				if word.value == 20:
					command.set_units(self.UNITS_INCHES)
				elif word.value == 21:
					command.set_units(self.UNITS_MM)

				if word.value == 28:
					# Home
					command.set_motion(self.MOTION_RAPID)
					command.set_position(0, 0.0)
					command.set_position(1, 0.0)

				if word.value == 90:
					command.set_postioning(self.POSTIONING_ABSOLUTE)
				elif word.value == 91:
					command.set_postioning(self.POSTIONING_RELATIVE)

				if word.value == 94:
					pass
					# raise FeatureNotSupported('G94	Feedrate.')

			if word.letter in ('N',):
				command.line_number = int(word.value)

			if word.letter in ('O',):
				command.state['program_name'] = word.value

			if word.letter in ('F',):
				command.set_feedrate(word.value)

			if word.letter in ('X', 'Y', 'Z'):
				if word.letter == 'Z':
					pass
				else:
					command.set_position({'X': 0, 'Y': 1, 'Z': 2}[word.letter], word.value)
				
			if word.letter in ('M',):
				if word.value == 117:
					command.set_message(word.message)
				elif word.value == 70:
					command.set_message(word.message)

			if word.letter in ('X', 'U', 'P'):
				command.set_dwell(word.value)

			if word.letter in ('R',):
				# Arc radius
				command.set_radius(word.value)
				
			if word.letter in ('I', 'J', 'K'):
				# Vectors that define the vector from the arc start point to the arc center point
				if word.letter == 'K':
					pass
				else:
					command.set_arc_position({'I': 0, 'J': 1, 'K': 2}[word.letter], word.value)

		return command


c = controller.Controller()
pol = plots.polar.PolarPlot(c)
plot = plots.cartesian.CartesianPlot(pol)
plot.set_origin(units.Cartesian(-350, -350))

machine_handle = Machine(plot, 3)
with open('exercise.gcode', 'r') as fh:
	machine_handle.parse_file(fh)

# machine_handle.parse_lines(test_three)

turtle_table.exit()
