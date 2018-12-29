# -*- coding: utf-8 -*-
#token.py
# This is one of the files that can be called by an npc_dialog, 
# The following code runs when a dialog has a pre rule of 'token'
# The syntax is 
# ["token", "tokenname", "possiblevalue1", "possiblevalue2", etc]
# To deliver a True verdict, the token tokenname must be set to one of the 
# 'possiblevalue' arguments. This will normally have been done 
# with a previous use of settoken
# The token is be kept with the player's data, and survive the reset
# of the map containing the NPC.
## DIALOGCHECK
## MINARGS 2
## MAXARGS 0
## .*
## .*
## ENDDIALOGCHECK

verdict = False
status = self.getStatus(args[0])
for value in args[1:]:
    if (status == value) or (value == "*"):
        verdict = True
        break
    else:
        pass

