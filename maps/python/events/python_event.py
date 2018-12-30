# global_event.py - this script handles all global events, except the plugin init (check plugin_init.py for that).
#
# This script merely looks and launches scripts in a specific subdirectory of /python/events/
# Only .py files are considered, you can simply rename a file to disable.
#
# This should simplify the separation of different Python things.

import Crossfire
import os

path = os.path.join(Crossfire.DataDirectory(), Crossfire.MapDirectory(), 'python/events', Crossfire.ScriptParameters())

if os.path.exists(path):
	scripts = os.listdir(path)

	for script in scripts:
		if (script.endswith('.py')):
			exec(open(os.path.join(path, script)).read())
