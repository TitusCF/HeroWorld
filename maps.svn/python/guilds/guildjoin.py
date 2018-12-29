# guildjoin.py - operates perilous chair for Hall of Joining in crossfire guilds
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
#

import Crossfire
import CFGuilds

def find_player(object):
    while (object.Type != 1) : #1 is type 'Player'
        object = object.Above
        if not object:
            return 0
    return object

activator=Crossfire.WhoIsActivator()
activatorname=activator.Name
map = activator.Map
whoami=Crossfire.WhoAmI()

guildname=Crossfire.ScriptParameters() # 1 is 'apply' event

if (guildname):
    guild = CFGuilds.CFGuild(guildname)
    #find players by coords
    ob=map.ObjectAt(9,16)
    player = find_player(ob)
    if player: # look for player
        charname=player.Name
        in_guild = CFGuilds.SearchGuilds(charname)
        if in_guild == 0:
            if guild.info(charname):
                #already a member
                message = '%s is already a member.' %charname
            else:
                guild.add_member(charname, 'Initiate')
                message = 'Added %s to the guild' %charname
        else:
            message = 'It appears that %s is already a member of the %s guild' %(charname, in_guild)
    else:
        message = 'No one is in the chair!'
else:
    print 'Guild Join Error: %s' %(guildname)
    message = 'Guild Join Error, please notify a DM'

whoami.Say(message)
