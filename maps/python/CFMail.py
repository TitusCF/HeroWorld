# CFMail.py - CFMail class
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

import os.path
import shelve

import Crossfire

class CFMail:

	maildb_file = os.path.join(Crossfire.LocalDirectory(),'crossfiremail')
	maildb = {}
	total = 0

	def __init__(self):
		self.maildb = shelve.open(self.maildb_file)

	def send(self, type, toname, fromname, message):
		# type: 1=mailscoll, 2=newsletter, 3=mailwarning
		if not toname in self.maildb:
			self.maildb[toname]=[[type,fromname,message]]
		else:
			temp=self.maildb[toname]
			temp.append([type,fromname,message])
			self.maildb[toname]=temp

	def receive(self, toname):
		if toname in self.maildb:
			elements=self.maildb[toname]
			del self.maildb[toname]
			return elements


	def countmail(self, toname):
		if toname in self.maildb:
			return len(self.maildb[toname])
		else:
			return 0
