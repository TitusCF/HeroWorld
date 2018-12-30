# -*- coding: utf-8 -*-
# QuestAdvance.py - A generic script to trigger quest progress
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
# This script is intended to be bound to event objects in order to speed quests along
# it must always be given the name of the quest as its first argument, followed by 
# any number of advance rules.
# an advance rule looks like
# 1>2
# or
# 2-4>5
# which will in the first case, move the stated quest from step 1 to step 2 
# in the second case, it will move the quest from step 2, 3 or 4 to step 5
# if no advance rule applies then nothing happens.
# something like 0>1 may be specified to start the quest.
# each advance rule should be separated by a space, there should be 
# no space within the individual rules.

import Crossfire

player = Crossfire.WhoIsActivator()

# if a spell was used, then the killer is the spell object, find the owner
if player.Type != Crossfire.Type.PLAYER:
  player = player.Owner

event = Crossfire.WhatIsEvent()
params = Crossfire.ScriptParameters()
args = params.split()
questname = args[0]
currentstep = player.QuestGetState(questname)    
for rule in args[1:]:
    condition, target = rule.split(">")
    if condition.find("-") == -1:
        startstep = int(condition)
        endstep = startstep
    else:
        startstep = int(condition.split("-")[0])
        endstep= int(condition.split("-")[1])
    if currentstep >= startstep and currentstep <= endstep:
    # update this quest
        if currentstep == 0:
            player.QuestStart(questname, int(target))
        else:
            player.QuestSetState(questname, int(target))
