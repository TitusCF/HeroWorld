# OFMail.py - OFMail class
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
    import xreadlines
    import os
    import fcntl
    import string

    class OFMail:

	maildb_dir  = '/usr/games/crossfire/var/crossfire/players/'
	maildb_file = '/mail'

	def openmail(self, name):
	    if os.access(self.maildb_dir + name + self.maildb_file, os.F_OK):
		mailfile = file(self.maildb_dir + name + self.maildb_file, 'r+')
	    else:
		mailfile = file(self.maildb_dir + name + self.maildb_file, 'w+')

	    fcntl.flock(mailfile, fcntl.LOCK_EX)
	    return mailfile

	def readmail(self, mailfile):
	    elements=[]
	    for line in xreadlines.xreadlines(mailfile):
		elements.append(string.split(line, None, 2))

	    return elements

	def closemail(self, mailfile):
	    fcntl.flock(mailfile, fcntl.LOCK_UN)
	    mailfile.close()

	def send(self, type, toname, fromname, message):
	    # type: 1=mailscoll, 2=newsletter, 3=mailwarning
	    mailfile=self.openmail(toname)
	    elements=self.readmail(mailfile)
	    elements.append([repr(type),fromname,message])
	    mailfile.seek(0)
	    element = []
	    for element in elements:
		    type, fromname, message = element
		    mailfile.write(type + ' ' + fromname + ' ' + string.strip(message) + '\n')

	    self.closemail(mailfile)

	def receive(self, toname):
	    mailfile=self.openmail(toname)
	    elements=self.readmail(mailfile)
	    os.remove(self.maildb_dir + toname + self.maildb_file)
	    self.closemail(mailfile)
	    return elements

	def countmail(self, toname):
	    mailfile=self.openmail(toname)
	    elements=self.readmail(mailfile)
	    self.closemail(mailfile)
	    return len(elements)

except:
    CFPython.Write('Mail Error :', activator)
    for line in traceback.format_exception(sys.exc_type, sys.exc_value, sys.exc_info()[2]):
	CFPython.Write(line, activator)

    raise

