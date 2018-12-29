# -*- coding: utf-8 -*-
# death_punisher.py - Punish players for killing some NPCs
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
# This script punishes players killing a NPC by making all other
# non aggressive monsters in the map aggressive.
#
# It should be called from a "death" event.

import Crossfire

def handle_death():
    map = Crossfire.WhoAmI().Map

    found = None

    for h in range(0, map.Height):
        for w in range(0, map.Width):
            top = map.ObjectAt(w, h)
            while top != None:
                if top.Monster == 1 and top.Unaggressive == 1:
                    found = top
                    found.Unaggressive = 0
                    break
                top = top.Above

    if found != None:
        found.Say("You are going to pay!")
    Crossfire.SetReturnValue(1)

handle_death()
