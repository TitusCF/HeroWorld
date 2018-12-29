# -*- coding: utf-8 -*-
# CFQuestStartAdvance - A generic script to make quest items undisposable.
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
# This script is intended to be bound to event_drop in the inventory of quest objects. 
# it must always be given the name of the quest as its first argument, followed by 
# the stage number it should act until.
# Until the specified stage in the specified quest is reached, the player will not be 
# permitted to drop the item, after the stage specified, any attempt to drop the 
# item will have it marked as startequip, causing it to disappear.
# This is to prevent quests being bypassed by trading keys, etc.

import Crossfire
item = Crossfire.WhoAmI()
player = Crossfire.WhoIsActivator()
args = Crossfire.ScriptParameters().split(' ')
questname = args[0]
stagenumber = int(args[1])
currentstep = player.QuestGetState(questname)
if currentstep == 0:
    Crossfire.SetReturnValue(0)
elif currentstep >= stagenumber:
    item.GodGiven = True
    Crossfire.SetReturnValue(0)
else:
    if item.Quantity == 1:
        player.Message("You consider dropping the "+ item.Name + " but then decide it would be better to hold on to it for now.")
    else:
        player.Message("You consider dropping the "+ item.NamePl + " but then decide it would be better to hold on to them for now.")
    Crossfire.SetReturnValue(1)
