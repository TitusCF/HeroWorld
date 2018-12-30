# -*- coding: utf-8 -*-
# npc_dialog.py - Dialog helper class
#
# Copyright (C) 2007 David Delbecq
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
#
# This is a simple script that makes use of CFNPCDialog.py and that receives
# parameters from a JSON inside the event message. 
#
# An example of a map file entry is:
#
# arch guildmaster
# name Sigmund
# msg
#
# endmsg
# x 11
# y 7
# arch event_say
# name start/sigmund.msg
# title Python
# slaying /python/dialog/npc_dialog.py
# end
# end
#
# see http://wiki.metalforge.net/doku.php/user:cavesomething:guide_to_quest_dialogs 
# for lots of detail on how to use this, and look at examples in test/quest_handling

import Crossfire
import os
from CFDialog import DialogRule, Dialog, IncludeRule
import cjson
import random

location = "defaultdialognamespace"

def parseJSON(filename, relpath):
    global location
    parameters = []
    for filenm in filename:
        if filenm[0] == "/":
            filepath = os.path.join(Crossfire.DataDirectory(),
                            Crossfire.MapDirectory(), filenm[1:])
        else:
            filepath = os.path.join(Crossfire.DataDirectory(),
                            Crossfire.MapDirectory(), relpath, filenm)
        try:
            f = open(filepath,'rb')
        except:
            Crossfire.Log(Crossfire.LogDebug, "Error loading NPC dialog %s" % filepath)
            raise
        else:
            Crossfire.Log(Crossfire.LogDebug, "Loading NPC dialog %s" % filepath)
            params = cjson.decode(f.read())
            f.close()
        if "location" in params:
            location = params["location"]
        for jsonRule in params["rules"]:
            if "include" in jsonRule:
                shouldinclude = 0
                if "pre" in jsonRule:
                    incldialog = Dialog(player, npc, location)
                    inclrule = IncludeRule(jsonRule["pre"])
                    # There will only ever be one 'pre' block for an include
                    shouldinclude = incldialog.matchConditions(inclrule)
                else:
                    shouldinclude =1
                newfiles = jsonRule["include"]
                if shouldinclude == 1:
                    # this isn't a 'real' rule, so we don't include it, but we do 
                    # include the results of parsing it.
                    parameters.extend(parseJSON(newfiles, os.path.dirname(filepath)))
                else:
                    Crossfire.Log(Crossfire.LogDebug, "Ignoring NPC dialog from %s, conditions not met" % newfiles)
            else:
                parameters.append(jsonRule)
    return parameters

npc = Crossfire.WhoAmI()
#event = Crossfire.WhatIsEvent()
player = Crossfire.WhoIsActivator()
if (Crossfire.ScriptParameters() != None):
    filename = Crossfire.ScriptParameters()
    dialogs = parseJSON([filename], '')
speech = Dialog(player, npc, location)
index = 0;

for jsonRule in dialogs:
    replies = None
    if jsonRule.has_key('replies'):
        replies = jsonRule['replies']
    rule = DialogRule(jsonRule["match"],
                      jsonRule["pre"],
                      jsonRule["msg"],
                      jsonRule["post"],
                      replies)
    speech.addRule(rule, index)
    index = index + 1

if speech.speak(Crossfire.WhatIsMessage()) == 0:
    # block the NPC for some time
    Crossfire.WhoAmI().WriteKey('talked_to', str(random.randint(3, 8)), 1);
    Crossfire.SetReturnValue(1)
