
import cProfile
import datetime
import functools
import inspect
import os
import subprocess
import pstats

def profile(*dec_args, **dec_kwargs):
	"""Profile the decorated function saving the stats file to disk.
		
	Args:
		Optional:
		name (String): Path to the stats file.
		print_stats (Boolean): Print the results of the stats file.
		load_image (Boolean): Graph the results of the stats file.
	"""
	
	function = None
	if len(dec_args) == 1 and callable(dec_args[0]):
		function = dec_args[0]
	name = dec_kwargs.get('name', None)
	load_image = dec_kwargs.get('load_image', False)
	print_stats = dec_kwargs.get('print_stats', False)
	
	def inner(func):

		@functools.wraps(func)
		def wrapper(*args, **kwargs):
			prof = cProfile.Profile()
			
			# Make the call
			retval = prof.runcall(func, *args, **kwargs)
			
			try:
				if name is None:
					filepath = inspect.getfile(func)
					funcname = func.__name__
					file_name = '{filepath}_{funcname}_{timestamp:%Y%m%d%H%M%S}.prof'.format(
						filepath=filepath,
						funcname=funcname,
						timestamp=datetime.datetime.now())
				else:
					file_name = name
				prof.dump_stats(file_name)

				p = pstats.Stats(file_name)
				if print_stats:
					p.strip_dirs().sort_stats('cumulative').print_stats(0.1)

				if load_image:
					image_file = dot_graph_profile(file_name)
					os.startfile(image_file)
			except Exception:
				raise
			return retval
		return wrapper

	# If we have not arguments passed into the decorator we can return the result
	# of inner now.
	if function:
		return inner(function)

	return inner


def get_file_from_env(filename, env_var):
	"""Search the environment var paths looking for a file."""

	for location in os.environ[env_var].split(';'):
		potential_location = os.path.join(location, filename)
		if os.path.isfile(potential_location):
			return potential_location


def dot_graph_profile(profile_filename, output_image_filename=None):
	"""Process the given stats file into a dot graphed image."""
	# Find our profile to dot script
	gprof2dot = 'C:/Users/krice/Google Drive/turtle/gprof2dot.py'
	dot = get_file_from_env('dot.exe', 'PATH')
	if not dot:
		print 'Dot was not found in the system path'
		return

	if output_image_filename is None:
		output_image_filename = profile_filename + '.png'

	# Run profile to dot script
	p1 = subprocess.Popen(
		['pythonw', gprof2dot, '-f', 'pstats', profile_filename],
		stdin=subprocess.PIPE,
		stdout=subprocess.PIPE,
		stderr=subprocess.PIPE)

	# Run dot
	si = subprocess.STARTUPINFO()
	si.dwFlags |= subprocess.STARTF_USESHOWWINDOW
	p2 = subprocess.Popen(
		['dot', '-Tpng', '-o', output_image_filename],
		stdin=p1.stdout,
		stdout=subprocess.PIPE,
		stderr=subprocess.PIPE,
		startupinfo=si)
	p1.stdout.close()
	p2.communicate()

	return output_image_filename