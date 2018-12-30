import Crossfire
import os.path
import sys

Crossfire.Log(Crossfire.LogDebug, "Running python initialize script.")
sys.path.insert(0, os.path.join(Crossfire.DataDirectory(), Crossfire.MapDirectory(), 'python'))

path = os.path.join(Crossfire.DataDirectory(), Crossfire.MapDirectory(), 'python/events/init')

if os.path.exists(path):
	scripts = os.listdir(path)

	for script in scripts:
		if (script.endswith('.py')):
			exec(open(os.path.join(path, script)).read())
