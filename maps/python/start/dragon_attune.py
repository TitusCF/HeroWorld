# dragon_attune.py
#
# Copyright 2008 by Lalo Martins
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
#
# This script sets the metabolism for a player dragon.
# It's meant to be run by the player changers in the hall of selection.

import Crossfire, os
animations = {
    2: 'pl_dragon_r',
    3: 'pl_dragon_blue',
    4: 'pl_dragon_bl',
    10: 'pl_dragon_g',
}
faces = {
    2: 'pl_dragon_r.171',
    3: 'pl_dragon_blue.171',
    4: 'pl_dragon_bl.132',
    10: 'pl_dragon_g.132',
}

changer = Crossfire.WhoAmI()
aname = Crossfire.ScriptParameters()
atype = getattr(Crossfire.AttackTypeNumber, aname.upper())
player = changer
while player and player.Archetype.Name != 'pl_dragon':
    player = player.Above
if player:
    force = player.CheckArchInventory('dragon_ability_force')
    force.Exp = atype
    player.Anim = animations[atype]
    player.Face = faces[atype]
    player.Title = '%s hatchling' % aname
    changer.Say("Your metabolism is now focused on me.")
