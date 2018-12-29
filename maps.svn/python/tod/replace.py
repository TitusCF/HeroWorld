# replace.py
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
# This script make the object is is attached to swap at
# given periods of day with a specifc object in the event's inventory
# To use it, give this event's parameter the name of
# period where the swap is active. Put in the inventiry the swapped object
# The swap can occur for objects in map and for object in other object
#
# exemple
#
# arch event_time
# title Python
# slaying /python/tod/replace_all_periods.py
# msg
# {
#     "when":["Morning","The Season of the Blizzard"]
#     "match":"all"
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
    Crossfire.Log(Crossfire.LogError,"Script replace_period.py didn't get a 'match' parameter. Only got %s" %parameters)
elif parameters["match"].lower() == "one":
    match=TimeOfDay().matchAny(parameters["when"]) != inverse
elif parameters["match"].lower() == "all":
    match=TimeOfDay().matchAll(parameters["when"]) != inverse
else:
    Crossfire.Log(Crossfire.LogError,"Script replace_period.py didn't get a 'match' parameter. Only got %s" %parameters)

if ( match != alreadymatched ):
    #Crossfire.Log(Crossfire.LogDebug, "replace_all_periods")
    event = Crossfire.WhatIsEvent()
    current = Crossfire.WhoAmI()
    future = event.Inventory
    # we do not want to repace a monster/anything useable in game by a subevent on the event.
    # seems for technical reasons, EVENT object have a destroy subevent. So let's just
    # ignore subevents
    while ( (future != None) & (future.Type == Crossfire.Type.EVENT_CONNECTOR)):
        future=future.Below
    if (future):
        #if (future.Below):
            #Crossfire.Log(Crossfire.LogDebug, "future.Below is %s" %future.Below.Name)
        #Crossfire.Log(Crossfire.LogDebug, "current is %s, future is %s, event is %s" %(current.Name, future.Name, event.Name))
        if (current.Env):
            #Crossfire.Log(Crossfire.LogDebug, "env mode")
            env = current.Env
            future.InsertInto(env)
            event.InsertInto(future)
            current.InsertInto(event)
            if (alreadymatched):
                event.Value=0
            else:
                event.Value=1
        elif (current.Map):
            #Crossfire.Log(Crossfire.LogDebug, "Map mode")
            mymap = current.Map
            x = current.X
            y = current.Y
            #Crossfire.Log(Crossfire.LogDebug, "inserting future %s in map" %future.Name)
            mymap.Insert(future,x,y)
            #Crossfire.Log(Crossfire.LogDebug, "inserting event %s in future" %event.Name)
            event.InsertInto(future)
            #Crossfire.Log(Crossfire.LogDebug, "inserting current %s in event" %current.Name)
            current.InsertInto(event)
            if (alreadymatched):
                event.Value=0
            else:
                event.Value=1
        #else:
            #Crossfire.Log(Crossfire.LogDebug, "neither env object nor map found")
