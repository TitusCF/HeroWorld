# filter.py
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
# This script make the event it is attached to (not global!)
# works only in specific moment of year/day. Periods are separated
# by comas. See wiki doc list of possible values
# exemple, to make an "apply" work only during Blizzard, new year and every morning:
#
# arch event_apply
# title Python
# slaying /python/tod/filter.py
# msg
# {
# "when":["The Season of New Year","The Season of the Blizzard","Morning"],
# "match" : "one",
# "inverse": true
# }
# endmsg
# end
import Crossfire
import string
from CFTimeOfDay import TimeOfDay
import cjson
parameters = cjson.decode(Crossfire.WhatIsEvent().Message)
inverse = "inverse" in parameters and parameters["inverse"] == True
Crossfire.SetReturnValue(not inverse)
if not "match" in parameters:
    Crossfire.Log(Crossfire.LogError,"Script filter.py didn't get a 'match' parameter. Only got %s" %parameters)
elif parameters["match"].lower() == "one":
    if TimeOfDay().matchAny(parameters["when"]):
        Crossfire.SetReturnValue(inverse)
elif parameters["match"].lower() == "all":
    if TimeOfDay().matchAll(parameters["when"]):
        Crossfire.SetReturnValue(inverse)
else:
    Crossfire.Log(Crossfire.LogError,"Script filter.py didn't get a 'match' parameter. Only got %s" %parameters)
