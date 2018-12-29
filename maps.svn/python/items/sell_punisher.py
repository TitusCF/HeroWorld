# -*- coding: utf-8 -*-
# sell_punisher.py - Punish players for selling items
#
# Copyright (C) 2010 Nicolas Weege
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
# This script punishes players trying to sell the item it is linked to.
# The item is then not sold.
#
# It should be called from a "selling" event.
#
# An optional "punish_sell_in" key can be put in the item (not the event)
# to specify a base path to punish selling it in maps which path starts with that value.
# For instance, to restrict selling to Scorn, use "punish_sell_in /scorn/".
#
# If no key is defined, trying to sell is punished in the whole world.

import Crossfire
import random

def sell_is_restricted():
    map = Crossfire.WhoAmI().ReadKey("punish_sell_in")
    if map == "":
        return 1

    current = Crossfire.WhoIsActivator().Map.Path

    if len(current) < len(map):
        return 0

    if map == current[0:len(map)]:
        return 1

    return 0

def handle_sell():
    if not sell_is_restricted():
        return

    guard = None

    map = Crossfire.WhoIsActivator().Map
    for h in range(0, map.Height):
        for w in range(0, map.Width):
            top = map.ObjectAt(w, h)
            while top != None:
                if top.Type == 69:
                    count = random.randint(2, 8)
                    while count > 0:
                        guard = Crossfire.CreateObjectByName("guard")
                        guard.StandStill = 0
                        guard.Unaggressive = 0
                        map.InsertAround(guard, w, h)
                        count = count - 1
                    break
                top = top.Above

    if guard != None:
        Crossfire.WhoIsActivator().Message("You thief!")
    Crossfire.SetReturnValue(1)

handle_sell()
