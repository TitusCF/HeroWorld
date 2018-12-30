# CFBoard.py - CFBoard class
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

class CFBoard:

	boarddb_file = os.path.join(Crossfire.LocalDirectory(),'crossfireboard')
	boarddb = {}
	total = 0

	def __init__(self):
		self.boarddb = shelve.open(self.boarddb_file)

	def write(self, boardname, author, message):
		if not boardname in self.boarddb:
			self.boarddb[boardname]=[[author,message]]
		else:
			temp=self.boarddb[boardname]
			temp.append([author,message])
			self.boarddb[boardname]=temp

	def list(self, boardname):
		if boardname in self.boarddb:
			elements=self.boarddb[boardname]
			return elements


	def delete(self, boardname, id):
		if boardname in self.boarddb:
			if id>0 and id<=len(self.boarddb[boardname]):
				temp=self.boarddb[boardname]
				temp.pop(id-1)
				self.boarddb[boardname]=temp
				return 1
		return 0

	def countmsg(self, boardname):
		if boardname in self.boarddb:
			return len(self.boarddb[boardname])
		else:
			return 0

	def getauthor(self, boardname, id):
		if boardname in self.boarddb:
			if id>0 and id<=len(self.boarddb[boardname]):
				author,message=self.boarddb[boardname][id-1]
				return author
