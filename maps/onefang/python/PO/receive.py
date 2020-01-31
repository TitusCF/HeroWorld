# recieve.py - Script for apply event of mailbox
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

    activatorname=CFPython.GetName(activator)
    whoami=CFPython.WhoAmI()

    mail = OFMail.OFMail()
    total = mail.countmail(activatorname)

    if total == 1:
	CFPython.Write('You have 1 mailscroll.', activator)
    elif total > 1:
	CFPython.Write('You have %s mailscrolls.'%total, activator)
    else:
	CFPython.Write('You haven\'t got any mail.', activator)

    if total > 0:
	elements = mail.receive(activatorname)
	element = []
        for element in elements:
		type, fromname, message = element
		if int(type)==1:
	                msgob = CFPython.CreateObjectInside('scroll', whoami)
	                CFPython.SetName(msgob,'mailscroll F: '+fromname+' T: '+activatorname)
			CFPython.Write('You got a mailscroll from %s.'%fromname, activator)
	                CFPython.SetMessage(msgob, message)
	                CFPython.SetValue(msgob, 0)
		elif int(type)==2:
	                msgob = CFPython.CreateObjectInside('note', whoami)
	                CFPython.SetName(msgob,'newspaper D: '+fromname)
			CFPython.Write('You got a newspaper from %s.'%fromname, activator)
	                CFPython.SetMessage(msgob, message)
	                CFPython.SetValue(msgob, 0)
		elif int(type)==3:
	                msgob = CFPython.CreateObjectInside('diploma', whoami)
	                CFPython.SetName(msgob,'mailwarning F: '+fromname+' T: '+activatorname)
			CFPython.Write('You got a mailwarning from %s.'%fromname, activator)
	                CFPython.SetMessage(msgob, message)
	                CFPython.SetValue(msgob, 0)
		else:
			CFPython.Write('ERROR: unknown mailtype', activator)

except:
    CFPython.Write('Receive Error :', activator)
    #for line in traceback.format_exception(sys.exc_type, sys.exc_value, sys.exc_info()[2]):
	#CFPython.Write(line, activator)

    raise
