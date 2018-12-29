# Script for say event of IPO message board
#
# Copyright (C) 2002 Joris Bontje
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
#
# help                - gives information about usage
#
#Updated to use new path functions in CFPython -Todd Mitchell

import Crossfire
import CFBoard
import string

board = CFBoard.CFBoard()

activator = Crossfire.WhoIsActivator()
activatorname = activator.Name
whoami = Crossfire.WhoAmI()

boardname = Crossfire.ScriptParameters()
if boardname:

	text = Crossfire.WhatIsMessage().split(' ', 1)

	if text[0] == 'help' or text[0] == 'yes':
		message='Help for %s\nList of commands:\n\n- list\n- write <message>\n- remove <id>\n'%boardname
		activator.Write(message)

	elif text[0] == 'write':
		if len(text) == 2:
			board.write(boardname, activatorname, text[1])
			activator.Write('Added to %s'%boardname)
		else:
			activator.Write('Usage "write <text>"')

	elif text[0] == 'list':
		total = board.countmsg(boardname)
		if total > 0:
			activator.Write('Content of %s:'%boardname)
			elements = board.list(boardname)
			element = []
			id = 1
			for element in elements:
				author, message = element
				activator.Write('<%d> (%s) %s'%(id, author, message))
				id = id+1
		else:
			activator.Write('%s is empty'%boardname)

	elif text[0] == 'remove':
		if len(text) == 2:
			index = int(text[1])
			if board.getauthor(boardname, index) == activatorname or activator.DungeonMaster:
				if board.delete(boardname, index):
					activator.Write('Removed from %s'%boardname)
				else:
					activator.Write('Doesn\'t exist on %s'%boardname)
			else:
				activator.Write('Access denied')
		else:
			activator.Write('Usage "remove <id>"')

	else:
		activator.Write('Do you need help?')

else:
	activator.Write('Board Error')
