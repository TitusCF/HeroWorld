# -*- coding: utf-8 -*-
#quest.py
# This is one of the files that can be called by an npc_dialog, 
# The following code runs when a dialog has a pre rule of 'quest'
# The syntax is ["quest", "questname", "queststage"]
# All arguments are required, questname must be a quest that is 
# defined by one of the .quests files. 
# Queststage can be in one of the following forms:
# 1) A number - eg "20" - The player must be at or past step 20 on questname
# 2) = A number - eg "=20" - The player must be at exactly step 20 on questname
# 3) = A range of numbers - eg "10-30" - The player must be betwen steps 10 and 30 on questname
# To deliver a True verdict, the player must be at the right stage in the quest.
## DIALOGCHECK
## MINARGS 2
## MAXARGS 2
## .*
## (=|)\d+(|-\d+)
## ENDDIALOGCHECK


questname = args[0]
stage = args[1]
if stage.find("-") == -1:
    if stage[0] == "=":
        startstep = int(stage[1:])
        endstep = startstep        
    else:
        startstep = int(stage)
        endstep = -1
else:
    startstep = int(stage.split("-")[0])
    endstep= int(stage.split("-")[1])

currentstep = character.QuestGetState(questname)
if currentstep < startstep:
    verdict = False
if endstep > -1 and currentstep > endstep:
    verdict = False