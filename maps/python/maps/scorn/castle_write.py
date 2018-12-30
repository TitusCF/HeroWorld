# -*- coding: utf-8 -*-
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

import Crossfire
import CFDataFile

player = Crossfire.WhoIsActivator()

nobledata = CFDataFile.CFData('scorn_nobility', ['rank', 'title'])
currentstep = player.QuestGetState("scorn/Aristocracy")
currentrecord = { '#' : player.Name, 'rank' : currentstep, 'title' : player.Title }
lastrecord = nobledata.get_record(player.Name)
Crossfire.Log(Crossfire.LogDebug, "castle_write: previous record %s, new record %s." % (lastrecord, currentrecord))
if lastrecord == 0:
    lastrecord =  { '#' : player.Name, 'rank' : -10, 'title' : 'The Default' }
if (currentrecord['rank'] == 0) or (currentrecord['rank'] == int(lastrecord['rank']) and currentrecord['title'] == lastrecord['title']):
    Crossfire.Log(Crossfire.LogDebug, "castle_write, no update needed for player %s." % player.Name)
else:
    Crossfire.Log(Crossfire.LogDebug, "castle_write, updating player %s, old state %s, new state %d" %(player.Name, lastrecord['rank'], currentstep))
    nobledata.put_record(currentrecord)
    player.Message("The castle sage scribbles as you walk past")



