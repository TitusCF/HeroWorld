# send.py - Script for close event of mailbox
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

import CFPython
import sys
import traceback
sys.path.append('%s/%s/python' %(CFPython.GetDataDirectory(),CFPython.GetMapDirectory()))
activator = CFPython.WhoIsActivator()

try:
    import OFMail
    import string
    from time import gmtime, strftime, time

    date = strftime("%a, %d %b %Y %H:%M:%S +0000", gmtime())
    activatorname=CFPython.GetName(activator)
    whoami=CFPython.WhoAmI()
    mail = OFMail.OFMail()
    idlist=[]

    inv = CFPython.CheckInventory(CFPython.WhoAmI(),"mailscroll")
    if inv != 0:
	while inv!=0:
		text=string.split(CFPython.GetName(inv))
		if text[0]=='mailscroll' and text[1]=='T:' and text[3]=='F:':
			idlist.append(inv)
			toname=text[2]
			fromname=text[4]
			message='%s %s\n' % (date, CFPython.GetMessage(inv)[:-1])
			CFPython.Write('mailscroll to '+toname+' sent.', activator)
			mail.send(1, toname, fromname, message)
		elif text[0]=='mailscroll' and text[1]=='F:' and text[3]=='T:':
			idlist.append(inv)
			fromname=text[2]
			toname=text[4]
			message=CFPython.GetMessage(inv)[:-1]
			mail.send(1, toname, fromname, message)
		else:
			print "ID: %d"%inv
			print "Name: "+CFPython.GetName(inv)
		inv=CFPython.GetNextObject(inv)

    inv = CFPython.CheckInventory(CFPython.WhoAmI(),"mailwarning")
    if inv != 0:
	while inv!=0:
		text=string.split(CFPython.GetName(inv))
		if text[0]=='mailwarning' and text[1]=='T:' and text[3]=='F:':
			idlist.append(inv)
			toname=text[2]
			fromname=text[4]
			message='%s %s\n' % (date, CFPython.GetMessage(inv)[:-1])
			CFPython.Write('mailwarning to '+toname+' sent.', activator)
			mail.send(3, toname, fromname, message)
		elif text[0]=='mailwarning' and text[1]=='F:' and text[3]=='T:':
			idlist.append(inv)
			fromname=text[2]
			toname=text[4]
			message=CFPython.GetMessage(inv)[:-1]
			mail.send(3, toname, fromname, message)
		else:
			print "ID: %d"%inv
			print "Name: "+CFPython.GetName(inv)
		inv=CFPython.GetNextObject(inv)

    for inv in idlist:
	CFPython.RemoveObject(inv)

except:
    CFPython.Write('Send Error :', activator)
    for line in traceback.format_exception(sys.exc_type, sys.exc_value, sys.exc_info()[2]):
	CFPython.Write(line, activator)

    raise
