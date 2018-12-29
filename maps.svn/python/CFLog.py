# CFLog.py - CFLog class
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

# Updated to use new path functions in CFPython -Todd Mitchell
#
# Updated to add new fields and functions (kick, muzzle)
# and rewritten to use plain text file storage (CFDataFile) instead of shelve.


import Crossfire

from time import localtime, strftime, time
from CFDataFile import CFDataFile, CFData

class CFLog:

    def __init__(self):
        logheader = ['Born', 'IP', 'Last_Login_Date', 'Login_Count', 'Kick_Count'
          , 'Last_Kick_Date', 'Muzzle_Count', 'Last_Muzzle_Date']
        self.log = CFData('Player_log', logheader)

    def create(self, name):
        date = strftime("%a, %d %b %Y %H:%M:%S %Z", localtime(time()))
        record={'#': name
        ,'Born':date
        ,'IP':'unknown'
        ,'Last_Login_Date':date
        ,'Login_Count':0
        ,'Kick_Count':0
        ,'Last_Kick_Date':'never'
        ,'Muzzle_Count':0
        ,'Last_Muzzle_Date':'never'}
        self.log.put_record(record)

    def remove(self, name):
        self.log.remove_record(name)

    def login_update(self, name, ip):
        date = strftime("%a, %d %b %Y %H:%M:%S %Z", localtime(time()))
        record = self.log.get_record(name)
        record['IP']=ip
        record['Last_Login_Date']=date
        record['Login_Count']=int(record['Login_Count'])+1
        self.log.put_record(record)

    def kick_update(self, name):
        date = strftime("%a, %d %b %Y %H:%M:%S %Z", localtime(time()))
        record = self.log.get_record(name)
        record['Kick_Count']=int(record['Kick_Count'])+1
        record['Last_Kick_Date']=date
        self.log.put_record(record)

    def muzzle_update(self, name):
        date = strftime("%a, %d %b %Y %H:%M:%S %Z", localtime(time()))
        record = self.log.get_record(name)
        record['Muzzle_Count']=int(record['Muzzle_Count'])+1
        record['Last_Muzzle_Date']=date
        self.log.put_record(record)

    def info(self, name):
        if name == '#':
            return 0
        record = self.log.get_record(name)
        if record:
            return record
        else:
            return 0
