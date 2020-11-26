"""Hook for the tracking system

When the server shuts down, all maps are unloaded.

This script calls the free_objects hook on all maps that are currently
loaded when the server goes down.
"""

import Crossfire

import tracking_system


for map_ in Crossfire.GetMaps():
    tracking_system.hook_freeobjects(map_)
