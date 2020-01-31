# bitchslap.py - give nightclub ticket to player
#
# Copyright 2003 David Seikel
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
# The author can be reached via e-mail at won_fang@yahoo.com.au
#


import CFPython
import sys
import traceback
sys.path.append('%s/%s/python' %(CFPython.GetDataDirectory(),CFPython.GetMapDirectory()))
activator = CFPython.WhoIsOther()

try:
    import string

    name = CFPython.GetName(activator)
    whoami=CFPython.WhoAmI()
    x=CFPython.GetXPosition(activator)
    y=CFPython.GetYPosition(activator)

    # Should figure out what to do for players that can't wear gloves.
    if not CFPython.CheckArchInventory(activator, 'mark_club'):
	CFPython.Write('The door bitch slaps a mark on your wrist.', activator)
	mark = CFPython.CreateObjectInside('mark_club', activator)
	CFPython.SetIdentified(mark, 1);
	CFPython.Apply(activator, mark, 33) #AP_APPLY | AP_IGNORE_CURSE

except:
    CFPython.Write('Bitch slap Error :', activator)
    for line in traceback.format_exception(sys.exc_type, sys.exc_value, sys.exc_info()[2]):
	CFPython.Write(line, activator)

    raise
