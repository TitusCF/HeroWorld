# CFDataFile.py - CFData classes
#
# Copyright (C) 2004 Todd Mitchell
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

import os
import Crossfire

class CFDataFile:
    '''Plain text storage for Crossfire data'''

    def __init__(self, datafile_name):
        '''make datafile paths for datafile 'object'
        - these all go in ../var/crossfire/datafiles to keep the local dir clean'''
        self.datafile_name = datafile_name
        self.filename = os.path.join((Crossfire.LocalDirectory()),'datafiles',datafile_name)

    def exists(self):
        '''checks for datafile - no need to load it yet'''
        if os.path.isfile(self.filename):
            return 1
        else:
            return 0

    def make_file(self, header):
        '''creates a datafile, making the column header from a list passed in'''
        try:
            file = open(self.filename,'wt')
        except:
            Crossfire.Log(Crossfire.LogError, "Can't create datafile %s" % self.datafile_name)
        else:
            temp = []
            for item in header:
                temp.append(str(item))
            contents = '#|%s\n' %('|'.join(temp))
            file.write(contents)
            file.close()
            Crossfire.Log(Crossfire.LogInfo, "New datafile created: %s" % self.datafile_name)

    def getData(self):
        '''Gets the formatted file as a dictionary
        The # key contains the column headers for the file and indicates the 'primary' key'''
        try:
            file = open(self.filename,'rt')
        except:
            raise Exception('Unable to read %s' % self.filename)
        else:
            temp = file.read().split('\n')
            file.close()
            contents = temp[:-1]
            DF = {}
            templist = []
            for list in contents:
                templist = list.split('|')
                DF[templist[0]] = templist[1:]
            return DF

    def putData(self, dic):
        '''Writes dictionary to formatted file - uses | character as a delimiter'''
        try:
            file = open(self.filename,'wt')
        except:
            raise Exception('Unable to open %s for writing' % self.datafile_name)
        else:
            header = dic['#']
            del dic['#']
            index = list(dic.keys())
            index.sort()
            contents = '#|%s\n' %('|'.join(header))
            file.write(contents)
            for entry in index:
                tmp = []
                stringlist = dic[entry]
                for item in stringlist:
                    tmp.append(str(item))
                temp = '%s|%s\n' %(entry, ('|'.join(tmp)))
                file.write(temp)
            file.close()

class CFData:
    '''CFData Object is basically a dictionary parsed from the datafile -
    serves to pass back and forth a dictionary containing the datafile header
    and the desired record to the caller - easier to read the '''

    def __init__(self, filename, header):
        self.header = header
        self.datafile = CFDataFile(filename)

        if self.datafile.exists():
            self.datadb = self.datafile.getData()
            if self.datadb['#'] != self.header:
                # see if header in calling object matches header in file
                # raise an alert but do nothing yet -
                # indicates possible upgrade of caller, will flesh this out later
                raise Exception('Header does not match!  You may need to fix the object or the datafile.')
        else:
            self.datafile.make_file(self.header)
            self.datadb = self.datafile.getData()

    def remove_record(self, name):
        if name in self.datadb:
            del self.datadb[name]
            self.datafile.putData(self.datadb)
            return 1
        else:
            return 0

    def exist(self, name):
        '''checks if a record exists given the primary key as "name"'''
        if name in self.datadb:
            return 1
        else:
            return 0

    def get_record(self, name):
        '''returns a small dictionary of the header and the record by "name"'''
        if self.exist(name):
            record = {}
            for header, item in zip(self.header,self.datadb[name]):
                record[header]=item
            record['#'] = name
            return record
        else:
            return 0

    def put_record(self, record):
        '''adds an line entry to the datafile'''
        name = record['#']
        del record['#']
        temp = []
        for item in self.header:
            temp.append(record[item])
        self.datadb[name]=temp
        self.datafile.putData(self.datadb)

    def get_keys(self):
        '''returns a sorted list of the primary keys (usually names) in the datafile'''
        keys = list(self.datadb.keys())
        keys.remove('#')
        keys.sort()
        return keys
