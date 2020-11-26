"""Hook for the tracking system"""

import Crossfire

import tracking_system


map_ = Crossfire.WhoAmI()
tracking_system.hook_freeobjects(map_)
