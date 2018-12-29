# -*- coding: utf-8 -*-
# Script for the ogre chief in /scorn/houses/doors_galore
#
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
# author: Nicolas for Khaleh

# This script waits for the ogre chief to be at a certain position
# then will move it to another while picking loot, then move it back
# to another position.
# Should be connected to the 'event_time' of the chief

import Crossfire

# Coordinates to be at to start the grab-move
start_x = 20
start_y = 4
# Where to go to while picking loot
grab_to_x = 20
grab_to_y = 8

# Where to go  back
to_final_x = 20
to_final_y = 7

# if you want to see what the chief is doing, set that to 1
chief_talks = 0

def move_grab(me):

    if chief_talks:
        me.Say('grab')

    below = me.Below
    while below.Floor == 0:
        take = below
        below = below.Below
        if chief_talks:
            me.Say('taking %s'%take.Name)
        me.Take(take)

    if me.X == grab_to_x and me.Y == grab_to_y:
        me.WriteKey('grab_state', 'final', 1)
        return

    me.Move(Crossfire.Direction.SOUTH)

def move_final(me):
    if chief_talks:
        me.Say('final')

    me.Move(Crossfire.Direction.NORTH)

    if me.X == to_final_x and me.Y == to_final_y:
        me.WriteKey('grab_state', 'finished', 1)
        if chief_talks:
            me.Say('finished')

def process():
    me = Crossfire.WhoAmI()
    state = me.ReadKey('grab_state')
    if state == 'finished':
        return

    if state == '':
        if me.X != start_x or me.Y != start_y:
            if chief_talks:
                me.Say('not at place')
            return

        me.WriteKey('grab_state', 'grabbing', 1)
        state = 'grabbing'
        # fall through

    Crossfire.SetReturnValue(1)

    if state == 'grabbing':
        move_grab(me)
        return

    if state == 'final':
        move_final(me)

process()
