# OFBank.py - OFBank class
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
    from time import gmtime, strftime, time
    import xreadlines
    import os
    import string

    class OFBank:

	logdb_dir  = '/usr/games/crossfire/var/crossfire/players/'
	logdb_file = '/bank'
	owner = 'onefang'

	def readlog(self, name):
	    elements={}
	    if os.access(self.logdb_dir + name + self.logdb_file, os.F_OK):
		logdb = file(self.logdb_dir + name + self.logdb_file)
		for line in xreadlines.xreadlines(logdb):
		    key, data = string.split(line, None, 1)
		    elements[key] = string.strip(data)

		logdb.close()

	    return elements

	def writelog(self, name, elements):
		logdb = file(self.logdb_dir + name + self.logdb_file, 'w')
		for key in elements.keys():
		    logdb.write(key + ' ' + elements[key] + '\n')
		    
		logdb.close()

	def create(self, name):
	    elements={}
	    date = strftime("%a, %d %b %Y %H:%M:%S +0000", gmtime())
	    elements['ip'] = 'unknown'
	    elements['date'] = date
	    elements['count'] = repr(1)
	    self.writelog(name, elements)

	def update(self, name, ip):
	    elements=self.readlog(name)
	    date = strftime("%a, %d %b %Y %H:%M:%S +0000", gmtime())
	    count=0
	    if elements.has_key('count'):
		count=int(elements['count'])
		
	    count+=1
	    elements['ip'] = ip
	    elements['date'] = date
	    elements['count'] =  repr(count)
	    self.writelog(name, elements)

	def remove(self, name):
	    if os.access(self.logdb_dir + name + self.logdb_file, os.F_OK):
		os.remove(self.logdb_dir + name + self.logdb_file)
	
	def exist(self, name):
	    if os.access(self.logdb_dir + name, os.F_OK):
		return 1
	    else:
		return 0

	def info(self, name):
	    elements=self.readlog(name)
	    ip = 'unknown'
	    date = 'unknown'
	    count = 0
	    if elements.has_key('ip'):
		ip = elements['ip']
	    if elements.has_key('date'):
		date = elements['date']
	    if elements.has_key('count'):
		count=int(elements['count'])

	    return ip, date, count

	def deposit(self, user, amount):
	    elements=self.readlog(user)
	    balance=0
	    if elements.has_key('balance'):
		balance=int(elements['balance'])
		
	    balance+=amount
	    elements['balance'] =  repr(balance)
	    self.writelog(user, elements)

	def withdraw(self, user, amount):
	    elements=self.readlog(user)
	    balance=0
	    if elements.has_key('balance'):
		balance=int(elements['balance'])
		
	    if balance>=amount:
		balance-=amount
		elements['balance'] = repr(balance)
		self.writelog(user, elements)
		return 1
		
	    return 0

	def getbalance(self,user):
	    elements=self.readlog(user)
	    balance=0
	    if elements.has_key('balance'):
		balance=int(elements['balance'])
		
            return balance

	def profit(self, amount):
	    elements=self.readlog(self.owner) # Should flock this.
	    balance=0
	    if elements.has_key('balance'):
		balance=int(elements['balance'])
		
	    balance+=amount/50
	    elements['balance'] =  repr(balance)
	    self.writelog(self.owner, elements)

except:
    CFPython.Write('Log Error :', activator)
    #for line in traceback.format_exception(sys.exc_type, sys.exc_value, sys.exc_info()[2]):
	#CFPython.Write(line, activator)

    raise
