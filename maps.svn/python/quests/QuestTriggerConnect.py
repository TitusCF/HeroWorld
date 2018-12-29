# -*- coding: utf-8 -*-
# QuestTriggerConnect.py - A generic script to trigger connections based on Quest progress
#
# Copyright (C) 2010 The Crossfire Development Team
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
# This script is intended to be bound to event objects in order to conditionally trigger
# connections based on the status of a quest, used correctly it can reduce the need for 
# markers to use checkinvs on.
# it must always be given the following arguments:
# name of the quest
# stage(s) to trigger on
# connection to trigger
# the quest must exist, and the connection must be present on the map.

import Crossfire

def trigger():
    player = Crossfire.WhoIsActivator()
    if player.Type != Crossfire.Type.PLAYER:
        return;
    params = Crossfire.ScriptParameters()
    args = params.split()
    questname = args[0]
    currentstep = player.QuestGetState(questname)
    condition = args[1]
    if condition.find("-") == -1:
        startstep = int(condition)
        endstep = startstep
    else:
        startstep = int(condition.split("-")[0])
        endstep= int(condition.split("-")[1])
    if currentstep >= startstep and currentstep <= endstep:
        Crossfire.Log(Crossfire.LogDebug, "QuestTriggerConnect.py: triggering connection number %s." % args[2])
        player.Map.TriggerConnected(int(args[2]), 1, player)

trigger()
