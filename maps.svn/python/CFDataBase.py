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
#Duplicated from CFBank.py and modified to store arbitrary python objects - Alestan 2010

import os.path
import shelve

import Crossfire

class CFDataBase:
    bankdb = {}

    def __init__(self, bankfile):
        self.bankdb_file = os.path.join(Crossfire.LocalDirectory(), bankfile)
        self.bankdb = shelve.open(self.bankdb_file, writeback=True)

    def store(self, name, value):
        
        self.bankdb[name]=value
        
        self.bankdb.sync()

        return 1

    def get(self,name):
        if name in self.bankdb:
                return self.bankdb[name]
        else:
                return 0

    def remove_record(self,name):
        if name in self.bankdb:
            del self.bankdb[name]
            Crossfire.Log(Crossfire.LogDebug, "%s CFDataBase record removed." %name)
            self.bankdb.sync()
            return 1
        else:
            return 0
