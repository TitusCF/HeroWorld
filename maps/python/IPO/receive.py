# receive.py - Script for apply event of mailbox
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
#Updated to use new path functions in CFPython -Todd Mitchell

import Crossfire
import CFMail
import string

activator=Crossfire.WhoIsActivator()
activatorname=activator.Name
whoami=Crossfire.WhoAmI()

mail = CFMail.CFMail()
total = mail.countmail(activatorname)

if total > 0:
	elements = mail.receive(activatorname)
	element = []
	for element in elements:
		type, fromname, message = element
		if type==1:
			msgob = whoami.CreateObject('scroll')
			msgob.Name='mailscroll F: '+fromname+' T: '+activatorname
			msgob.NamePl='mailscrolls F: '+fromname+' T: '+activatorname
			msgob.Message=message
			msgob.Value=0
		elif type==2:
			msgob = whoami.CreateObject('note')
			msgob.Name='newspaper D: '+fromname
			msgob.NamePl='newspapers D: '+fromname
			msgob.Message=message
			msgob.Value=0
		elif type==3:
			msgob = whoami.CreateObject('diploma')
			msgob.Name='mailwarning F: '+fromname+' T: '+activatorname
			msgob.NamePl='mailwarnings F: '+fromname+' T: '+activatorname
			msgob.Message=message
			msgob.Value=0
		else:
			Crossfire.Log(Crossfire.LogError, 'ERROR: unknown mailtype\n')

if total == 1:
	activator.Write('You got 1 mail.')
elif total > 1:
	activator.Write('You got %s mails.'%total)
else:
	activator.Write('You haven\'t got any mail.')
