# -*- coding: utf-8 -*-
#level.py
# This is one of the files that can be called by an npc_dialog, 
# The following code runs when a dialog has a pre rule of 'level'
# The syntax is ["level", "levelrequired"]
# To deliver a True verdict, the player must be at or above level levelrequired
## DIALOGCHECK
## MINARGS 1
## MAXARGS 1
## \d
## ENDDIALOGCHECK

targetlevel = int(args[0])
if len(args) == 1:
    if character.Level < targetlevel:
        verdict = False
else:
    verdict = False
    #TODO - add support for checking the level of individual skills
