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
params = Crossfire.ScriptParameters()
Crossfire.SetReturnValue(1)

nobledata = CFDataFile.CFData('scorn_nobility', ['rank', 'title'])
args = params.split()
targetrank = int(args[0])
suffix=""
prefix=""
if targetrank >= 90:
    rankname = "Prince"
    prefix = "His Royal Highness "
elif targetrank >= 80:
    rankname = "Archduke"
    prefix = "His Highness "
elif targetrank >= 70:
    rankname = "Duke"
    prefix = "His Grace "
elif targetrank >= 60:
    rankname = "Count"
    prefix = "His Excellency "
elif targetrank >= 50:
    rankname = "Marquis"
    prefix = "The Most Honourable "
elif targetrank >= 40:
    rankname = "Earl"
    prefix = "The Right Honourable "
elif targetrank >= 30:
    rankname = "Baron"
    prefix = "The Much Honoured "
elif targetrank >= 20:
    rankname = "Baronet"
    prefix = "Sir "
    suffix = " Bt."
elif targetrank >= 10:
    rankname = "Knight"
    prefix = "Sir "

player.Message("Herein are recorded those who have obtained the rank of " + rankname + " in the Kingdom of Scorn:")
playercount = 0
print nobledata.get_keys()
for noble in nobledata.get_keys():
    print noble
    record = nobledata.get_record(noble)
    print record
    if int(record['rank']) == targetrank:
        player.Message(prefix+noble+" "+record['title']+suffix)
        playercount +=1
if playercount == 0:
    player.Message("... The list is empty.")
