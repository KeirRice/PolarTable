#! /usr/bin/python
# -*- coding: utf-8 -*-

# Imports ####################################################################

from turtle import Turtle, Screen
import turtle as turtle_global
import Tkinter as tk

from constants import TABLE_RADIUS_MM
from controller import Controller
from plots.cartesian import CartesianPlot
from plots.polar import PolarPlot
from units import *


# Monkey Patch ###############################################################

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


# Setup Turtle ###############################################################

window = Pixels(800)
turtle_global.getscreen().setup(width=int(window), height=int(window), startx=None, starty=None)
turtle_global.screensize(int(window) - 20, int(window) - 20, "black")
turtle_global.hideturtle()

turtle = Turtle()
screen_size = Pixels(turtle.screen.screensize()[0])
world_size = CartesianResolution(900, 900)
Distance.set_conversion_factor(screen_size, world_size.x)
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


# Tests ######################################################################

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

	plot.move_fast(Cartesian(TABLE_RADIUS_MM, TABLE_RADIUS_MM))
	plot.draw_circle(Cartesian(440.0, 0.0), Millimeters(50.0))


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
