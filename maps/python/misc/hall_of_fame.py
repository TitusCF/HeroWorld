# -*- coding: utf-8 -*-
# hall_of_fame.py - A script handling 'hall of fame' lists
#
# Copyright (C) 2010-2012 The Crossfire Development Team
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
# This script is intended to be bound to event objects, generally the apply one.
# If the item this script is bound to is a SIGN, then it will display
# its list of players, preceded by the own item's message (as header).
# For any other item, the player is added to the list, with her title. If the event
# object itself has a message, it will be displayed to the player if she is added
# to the list.
#
# The script's parameters is the internal name of the quest.

import Crossfire
from CFDataFile import CFData

player = Crossfire.WhoIsActivator()
me = Crossfire.WhoAmI()
event = Crossfire.WhatIsEvent()
name = 'hall_of_fame_' + Crossfire.ScriptParameters()

header = [ 'title' ]
file = CFData(name, header)

if me.Type == Crossfire.Type.SIGN:
  message = me.Message + "\n"
  keys = file.get_keys()
  for name in keys:
    message = message + name + " " + file.get_record(name)['title'] + "\n"
  if len(keys) == 0:
    message = message + "No one is recorded, will you attempt to be the first?"
  player.Write(message)
  Crossfire.SetReturnValue(1)
else:
  record = file.get_record(player.Name)
  if record == 0:
    file.put_record({ '#' : player.Name, 'title' : player.Title })
    if event.Message != None:
      player.Write(event.Message)


