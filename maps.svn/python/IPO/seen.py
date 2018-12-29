# Script for seen event
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
#Updated to use new path functions in CFPython, and broken into tiny bits by -Todd Mitchell
#
# seen            - tells player information from logger

import Crossfire
import string
import CFLog

activator=Crossfire.WhoIsActivator()
activatorname=activator.Name
whoami=Crossfire.WhoAmI()
isDM=activator.DungeonMaster
x=activator.X
y=activator.Y

log = CFLog.CFLog()
text = Crossfire.WhatIsMessage().split()

if text[0] == 'seen':
    if len(text)==2:
        record = log.info(text[1])
        if record:
            if isDM:
                message = "I have seen '%s' %d times.\nI saw them last coming from\nIP: %s\non %s." % (text[1], int(record['Login_Count']), record['IP'], record['Last_Login_Date'])
            else:
                message = "I have seen '%s' %d times.\nI saw them last at %s." % (text[1], int(record['Login_Count']), record['Last_Login_Date'])
        else:
            message = "I have never seen '%s'." % text[1]
    else:
        message = 'Usage "seen <player>"'

elif text[0] == 'help' or text[0] == 'yes':
    if isDM:
        message = "How can I help you? Here is a quick list of commands:\nseen, info, muzzlecount, lastmuzzle, kickcount, lastkick"
    else:
        message = "I have seen just about everybody - go ahead and ask me."

elif text[0] == 'muzzlecount' and isDM:
    if len(text)==2:
        record = log.info(text[1])
        if record:
            message = "%s has been muzzled %d times" % (text[1],int(record['Muzzle_Count']))
        else:
            message = "I have no knowledge of '%s'." % text[1]
    else:
        message = 'Usage "muzzlecount <player>"'

elif text[0] == 'lastmuzzle' and isDM:
    if len(text)==2:
        record = log.info(text[1])
        if record:
            message = "%s was last muzzled on %s" % (text[1],record['Last_Muzzle_Date'])
        else:
            message = "I have no knowledge of '%s'." % text[1]
    else:
        message = 'Usage "muzzlestatus <player>"'

elif text[0] == 'kickcount' and isDM:
    if len(text)==2:
        record = log.info(text[1])
        if record:
            message = "%s has been kicked %d times" % (text[1],int(record['Kick_Count']))
        else:
            message = "I have no knowledge of '%s'." % text[1]
    else:
        message = 'Usage "kickcount <player>"'

elif text[0] == 'lastkick' and isDM:
    if len(text)==2:
        record = log.info(text[1])
        if record:
            message = "%s was last kicked out on %s" % (text[1],record['Last_Kick_Date'])
        else:
            message = "I have no knowledge of '%s'." % text[1]
    else:
        message = 'Usage "lastkick <player>"'

elif text[0] == 'info' and isDM:
    if len(text)==2:
        record = log.info(text[1])
        if record:
            message = "%s" % (record)
        else:
            message = "I have no knowledge of '%s'." % text[1]
    else:
        message = 'Usage "info <player>"'
else:
    message = "Do you need help?"

whoami.Say(message)
Crossfire.SetReturnValue(1)
