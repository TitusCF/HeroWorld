# CFGuilds.py - classes for crossfire guilds
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
# The author can be reached via e-mail at temitchell@sourceforge.net

import Crossfire

from time import localtime, strftime, time
import os
from CFDataFile import CFDataFile, CFData

def GuildUpdate():
    GuildList = os.path.join(Crossfire.DataDirectory(),Crossfire.MapDirectory(),'templates','guild','GuildList')
    try:
            guildfile = open(GuildList,'r')
            guildlisting = guildfile.read().split('\n')
            guildfile.close()
            guildlisting.pop()
    except:
        Crossfire.Log(Crossfire.LogError,'No GuidList file.  Please check %s' %GuildList)
    if (guildlisting):
        Crossfire.Log(Crossfire.LogDebug, '%s' %guildlisting)
        for guild in guildlisting:
            if not CFGuildHouses().info(guild):
                if CFGuildHouses().add_guild(guild):
                    Crossfire.Log(Crossfire.LogInfo,'New Guild: %s' %guild)

def SearchGuilds(player):
    guildlist = CFGuildHouses().list_guilds()
    for guild in guildlist:
        memberlist = CFGuild(guild).list_members()
        for member in memberlist:
            if member == player:
                return guild
    return 0

class CFGuildHouses:
    '''Inter-Guild management class - loads guild from GuildList'''

    def __init__(self):
        guildhousesheader= ['Founded_Date', 'Points', 'Status', 'Quest_points']
        self.status=['active', 'suspended', 'probation', 'inactive']
        self.guildhouselist = CFData('GuildHouses_log', guildhousesheader)
        self.pointscap = 2000000000
        self.qpcap = 2000000000

    def info(self, name):
        record = self.guildhouselist.get_record(name)
        if record:
            return record
        else:
            return 0

    def list_guilds(self):
          return self.guildhouselist.get_keys()

    def add_guild(self, name):
        record={'#' : name
                    ,'Founded_Date' : 'never'
                    ,'Points' : 0
                    ,'Status' : 'inactive'
                    ,'Quest_points': 0
                    }
        try:
            self.guildhouselist.put_record(record)
            return 1
        except:
            return 0

    def establish(self, name):
        record = self.info(name)
        date = strftime("%a, %d %b %Y", localtime(time()))
        if record:
            record['Founded_Date'] = date
            record['Status'] = 'active'
            self.guildhouselist.put_record(record)
            return 1
        else:
            return 0

    def change_status(self, name, status):
        record = self.info(name)
        if record:
            if self.status.count(status) != 0:
                record['Status'] = status
                self.guildhouselist.put_record(record)
                return 1
            else:
                return 0
        else:
            return 0

    def add_questpoints(self, name, number):
        record = self.info(name)
        if record:
            temp = int(record['Quest_points'])
            if temp + number >= self.qpcap:
                temp = self.qpcap
            else:
                temp = temp + number
            record['Quest_points'] = temp
            self.guildhouselist.put_record(record)
            return 1
        else:
            return 0

    def update_points(self, name, number):
        record = self.info(name)
        if record:
            temp = int(record['Points'])
            if temp + number >= self.pointscap:
                temp = self.pointscap
            else:
                temp = temp + number
            record['Points'] = temp
            self.guildhouselist.put_record(record)
            return 1
        else:
            return 0

class CFGuild:

    def __init__(self, guildname):
        self.guildname = guildname
        guildheader = ['Join_date', 'Rank', 'Status', 'Dues', 'Demerits', 'Quest_points']
        self.ranks=['Initiate', 'Novice', 'Guildman', 'Journeyman', 'Master', 'GuildMaster']
        self.status=['suspended', 'probation', 'good']
        self.duescap = 1000000 #arbitrary limit on dues (if you change it remember it currently is an int)
        self.demeritcap = 100 #arbitrary limit on demerits
        self.qpcap = 100
        guild_name = '%s_log' %self.guildname
        self.guildlist = CFData(guild_name, guildheader)

    def add_member(self, name, rank):
        date = strftime("%a, %d %b %Y", localtime(time()))
        record={'#' : name
        ,'Join_date' : date
        ,'Rank' : rank
        ,'Status' : 'good'
        ,'Dues' : 0
        ,'Demerits' : 0
        ,'Quest_points': 0
        }
        try:
            self.guildlist.put_record(record)
            return 1
        except:
            return 0

    def remove_member(self, name):
        try:
            self.guildlist.remove_record(name)
            return 1
        except:
            return 0

    def info(self, name):
        record = self.guildlist.get_record(name)
        if record:
            return record
        else:
            return 0

    def list_members(self):
        return self.guildlist.get_keys()

    def count_members(self):
        number = len(self.guildlist.get_keys())
        return number

    def add_demerits(self, name, number):
        record = self.info(name)
        if record:
            temp = int(record['Demerits'])
            if temp + number >= self.demeritcap:
                temp = self.demeritcap
            else:
                temp = temp + number
            record['Demerits'] = temp
            self.guildlist.put_record(record)
            return 1
        else:
            return 0

    def remove_demerits(self, name, number):
        record = self.info(name)
        if record:
            temp = int(record['Demerits'])
            if temp - number <=0:
                temp = 0
            else:
                temp = temp - number
            record['Demerits'] = temp
            self.guildlist.put_record(record)
            return 1
        else:
            return 0

    def add_questpoints(self, name, number):
        record = self.info(name)
        if record:
            temp = int(record['Quest_points'])
            if temp + number >= self.qpcap:
                temp = self.qpcap
            else:
                temp = temp + number
            record['Quest_points'] = temp
            self.guildlist.put_record(record)
            return 1
        else:
            return 0

    def pay_dues(self, name, number):
        record = self.info(name)
        if record:
            temp = int(record['Dues'])
            if temp + number >= self.duescap:
                temp = self.duescap
            else:
                temp = temp + number
            record['Dues'] = temp
            self.guildlist.put_record(record)
            # add points to the guild record
            CFGuildHouses().update_points(self.guildname,number)
            return 1
        else:
            return 0

    def clear_dues(self, name, number):
        record = self.info(name)
        if record:
            record['Dues'] = 0
            self.guildlist.put_record(record)
            return 1
        else:
            return 0

    def change_status(self, name, status):
        record = self.info(name)
        if record:
            if self.status.count(status) != 0:
                record['Status'] = status
                self.guildlist.put_record(record)
                return 1
            else:
                return 0
        else:
            return 0

    def promote_member(self, name):
        record = self.info(name)
        if record:
            currentrank = record['Rank']
            if currentrank != 'GuildMaster':
                ranknum = self.ranks.index(currentrank)
                newrank = ranknum+1
                record['Rank'] = self.ranks[newrank]
                self.guildlist.put_record(record)
                return 1
            else:
                return 0
        else:
            return 0

    def demote_member(self, name):
        record = self.info(name)
        if record:
            currentrank = record['Rank']
            if currentrank != 'Initiate':
                ranknum = self.ranks.index(currentrank)
                Crossfire.Log(Crossfire.LogDebug,  "ranknum = %d"%ranknum)
                newrank = ranknum-1
                record['Rank'] = self.ranks[newrank]
                self.guildlist.put_record(record)
                return 1
            else:
                return 0
        else:
            return 0
