# -*- coding: utf-8 -*-
#questdone.py
# This is one of the files that can be called by an npc_dialog,
# The following code runs when a dialog has a pre rule of 'questdone'
# The syntax is ["questdone", "questname"]
# All arguments are required, questname must be a quest that is
# defined by one of the .quests files.
# To deliver a True verdict, the player must have completed at least once the quest.
## DIALOGCHECK
## MINARGS 1
## MAXARGS 1
## .*
## ENDDIALOGCHECK


questname = args[0]
verdict = character.QuestWasCompleted(questname)
