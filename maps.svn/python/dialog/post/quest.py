# -*- coding: utf-8 -*-
#quest.py
# This is one of the files that can be called by an npc_dialog, 
# The following code runs when a dialog has a post rule of 'quest'
# The syntax is ["quest", "questname", "queststage"]
# All arguments are required, questname must be a quest that is 
# defined by one of the .quests files queststage must be a step 
# number in that quest
## DIALOGCHECK
## MINARGS 2
## MAXARGS 2
## .*
## \d+
## ENDDIALOGCHECK

questname = args[0]
stage = args[1]
if character.QuestGetState(questname) == 0:
    Crossfire.Log(Crossfire.LogDebug, "CFDialog: starting quest: %s at stage %s for character %s" %(questname, stage, character.Name))
    character.QuestStart(questname, int(stage))
elif int(stage) > character.QuestGetState(questname):
    Crossfire.Log(Crossfire.LogDebug, "CFDialog: advancing quest: %s to stage %s for character %s" %(questname, stage, character.Name ))
    character.QuestSetState(questname, int(stage))
else:
    Crossfire.Log(Crossfire.LogError, "CFDialog: Tried to advance a quest backwards.")