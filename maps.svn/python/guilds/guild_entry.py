# Script for entering guild houses
#
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
# authors: majorwoo josh@woosworld.net, Avion temitchell@sourceforge.net

import Crossfire
import CFGuilds

import sys

import string

activator=Crossfire.WhoIsActivator()
activatorname=activator.Name
mymap = activator.Map
x=32
y=16
x1=36
y1=20
activatorx=activator.X
activatory=activator.Y
whoami=Crossfire.WhoAmI()

from CFGuildClearance import CheckClearance
Params=Crossfire.ScriptParameters().split()


guild = CFGuilds.CFGuild(Params[0])
guildname=Params[0]

text = Crossfire.WhatIsMessage().split()
guildrecord = CFGuilds.CFGuildHouses().info(guildname)
found = 0
if text:
        if text[0].upper() == 'ENTER':
                print guildrecord
                if guildrecord['Status'] == 'inactive':
                        message = 'This guild is currently inactive and available to be bought.'

                elif guildrecord['Status'] == 'suspended':
                        message = 'This guild is currently under suspension.\nPlease see a DM for more information'

                else:
                        if guildrecord['Status'] == 'probation':
                                activator.Write('This guild is currently under probation.\nPlease see a DM for more information')

                if (CheckClearance(Params,activator)):
                    #check their status
                
                
                        message = 'Entry granted for %s' %activatorname
                        mymap.TriggerConnected(int(Params[2]),1,activator)
                else:
                    message = 'You try my patience %s.  BEGONE!' %activatorname
                    activator.Teleport(mymap,int(x),int(y)) #teleport them

        elif text[0].upper() == 'BUY' and whoami.Name=="Guardian":
                if guildrecord['Status'] == 'inactive':
                        in_guild = CFGuilds.SearchGuilds(activatorname)
                        if in_guild == 0:
                                x = 30
                                y = 19
                                message = "Proceed, but know ye that three are required to found a guild and the cost is high"
                                activator.Teleport(mymap,int(x),int(y)) #teleport them
                        else:
                                x = 30
                                y = 19
                                message = "Proceed, but know ye that three are required to found a guild and the cost is high.\n  Note, you are already a member of a guild, some servers may prohibit being in multiple guilds.  Proceed at your own risk."
                                activator.Teleport(mymap,int(x),int(y)) #teleport them
                else:
                        message = 'This guild is already owned.'
        elif whoami.Name=="Guardian":
                message = 'This is the entry to the great %s guild.  Enter or begone!' %guildname
        else:
                message = "Say enter to request entry."
else:
    message = 'Guild Guardian Error, please notify a DM'

whoami.Say(message)

