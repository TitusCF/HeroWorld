# sleep.py
#
# Copyright 2007 by David Delbecq
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#
# Uses JSON notation for parameters
#
# This script makes the monster it is attached to (use EVENT_TIME to attach it)
# sleep at certain periods of the day/month/year. Note that it will
# triggers only once per period change. That mean a sleeping monster awaken
# by a PC  will not all of sudden go to sleep. Also, remember that most moster
# awaken and attack when a player is nearby. So effects of this script might be
# limited.
#
# exemple
#
# arch event_time
# title Python
# slaying /python/tod/sleep.py
# msg
# {
#     "when":["Dawn","Night"]
#     "match":"one"
# }
# endmsg
# arch beholder
# end
# end
#

import Crossfire
import string
from CFTimeOfDay import TimeOfDay
import cjson
event = Crossfire.WhatIsEvent()
parameters = cjson.decode(event.Message)
alreadymatched = (event.Value!=0)
inverse = "inverse" in parameters and parameters["inverse"] == True
match = False
if not "match" in parameters:
    Crossfire.Log(Crossfire.LogError,"Script sleep.py didn't get a 'match' parameter. Only got %s" %parameters)
elif parameters["match"].lower() == "one":
    match=TimeOfDay().matchAny(parameters["when"]) != inverse
elif parameters["match"].lower() == "all":
    match=TimeOfDay().matchAll(parameters["when"]) != inverse
else:
    Crossfire.Log(Crossfire.LogError,"Script sleep.py didn't get a 'match' parameter. Only got %s" %parameters)

if ( match != alreadymatched ):
    Crossfire.Log(Crossfire.LogDebug, "sleep")
    event = Crossfire.WhatIsEvent()
    current = Crossfire.WhoAmI()
    if (current):
        if (alreadymatched):
            event.Value=0
            current.Sleeping=0
            Crossfire.Log(Crossfire.LogDebug, "Awaken %s" %current.Name)
        else:
            event.Value=1
            current.Sleeping=1
            Crossfire.Log(Crossfire.LogDebug, "Put %s to sleep" %current.Name)
