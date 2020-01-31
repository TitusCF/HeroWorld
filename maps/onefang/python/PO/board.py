# Script for say event of IPO message board
#
# Copyright (C) 2002 Joris Bontje
# Modifications to use text file instead of DB Copyright 2003 David Seikel
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
# The author can be reached via e-mail at jbontje@suespammers.org
# The author can be reached via e-mail at won_fang@yahoo.com.au
#
# help                - gives information about usage


import CFPython
import sys
import traceback
sys.path.append('%s/%s/python' %(CFPython.GetDataDirectory(),CFPython.GetMapDirectory()))
activator = CFPython.WhoIsActivator()

boarddb_dir = '/usr/games/crossfire/var/crossfire/board/'

try:
    import string
    import traceback
    import xreadlines
    import os

    def readboard(boardname):
	elements=[]
	if os.access(boarddb_dir + boardname, os.F_OK):
	    boarddb = file(boarddb_dir + boardname)
	    for line in xreadlines.xreadlines(boarddb):
		elements.append(string.split(line, None, 1))

	    boarddb.close()

	return elements

    activatorname=CFPython.GetName(activator)
    whoami=CFPython.WhoAmI()
    boardname=CFPython.GetEventOptions(whoami,6) # 6 is say event
    if (boardname):
	text = string.split(CFPython.WhatIsMessage(), None, 1)
	if len(text)==0:
	    text=['']

	if text[0] == 'help' or text[0] == 'yes':
	    message='Help for %s message board.\nList of commands:\n\n- list\n- write <message>\n- remove <id>\n'%boardname
	    CFPython.Write(message, activator)

	elif text[0] == 'write':
	    if len(text)==2:
		if not os.access(boarddb_dir, os.F_OK):
		    os.makedirs(boarddb_dir)

		boarddb = file(boarddb_dir + boardname, 'a')
		boarddb.write(activatorname + ' ' + text[1] + '\n')
		boarddb.close()
		CFPython.Write('Added to %s message board.'%boardname, activator)
	    else:
		CFPython.Write('Usage "write <text>"', activator)

	elif text[0] == 'list':
	    elements=readboard(boardname)
	    if len(elements)>0:
		CFPython.Write('Content of %s message board:'%boardname, activator)
		element = []
		id = 1
		for element in elements:
		    author, message = element
		    CFPython.Write('<%d> (%s) %s'%(id,author,message), activator)
		    id=id+1
		    
	    else:
		CFPython.Write('%s message board is empty.'%boardname, activator)

	elif text[0] == 'remove':
	    if len(text)==2:
		item = int(text[1]) - 1
		elements=readboard(boardname)
		if item>=len(elements) or item<0:
		    CFPython.Write('Doesn\'t exist on %s message board.'%boardname, activator)
		else:
		    if elements[item][0]==activatorname or CFPython.IsDungeonMaster(activator):
			del elements[item]
			boarddb = file(boarddb_dir + boardname, 'w')
			element = []
			for element in elements:
			    author, message = element
			    boarddb.write(author + ' ' + message)
		    
			boarddb.close()
			CFPython.Write('Message removed from %s message board.'%boardname, activator)
		    else:
			CFPython.Write('Access denied.', activator)

	    else:
		CFPython.Write('Usage "remove <id>"', activator)

	else:
	    CFPython.Write('Do you need help?', activator)

    else:
	CFPython.Write('Board Error.', activator)

except:
    CFPython.Write('Board Error :', activator)
    #for line in traceback.format_exception(sys.exc_type, sys.exc_value, sys.exc_info()[2]):
	#CFPython.Write(line, activator)

    raise
