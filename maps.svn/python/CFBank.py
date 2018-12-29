# CFBank.py - CFBank class
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

class CFBank:
	bankdb = {}

	def __init__(self, bankfile):
		self.bankdb_file = os.path.join(Crossfire.LocalDirectory(), bankfile)
		self.bankdb = shelve.open(self.bankdb_file, writeback=True)

	def deposit(self, user, amount):
		if not user in self.bankdb:
			self.bankdb[user]=amount
		else:
			temp=self.bankdb[user]
			self.bankdb[user]=temp+amount
		self.bankdb.sync()

	def withdraw(self, user, amount):
		if user in self.bankdb:
			if (self.bankdb[user] >= amount):
				temp=self.bankdb[user]
				self.bankdb[user]=temp-amount
				self.bankdb.sync()
				return 1
		return 0

	def getbalance(self,user):
		if user in self.bankdb:
				return self.bankdb[user]
		else:
				return 0

	def remove_account(self,user):
		if user in self.bankdb:
			del self.bankdb[user]
			Crossfire.Log(Crossfire.LogDebug, "%s's bank account removed." %user)
			self.bankdb.sync()
			return 1
		else:
			return 0
