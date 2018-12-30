# -*- coding: utf-8 -*-
# settoken.py
# This is one of the files that can be called by an npc_dialog, 
# The following code runs when a dialog has a post rule of 'settoken'
# The syntax is ["settoken", "tokenname", "valuetosetto"]
# this can then be checked by a token condition that looks for the 
# value of the token
# The token will be kept with the player's data, and survive the reset
# of the map containing the NPC.
## DIALOGCHECK
## MINARGS 2
## MAXARGS 2
## .*
## .*
## ENDDIALOGCHECK

self.setStatus(args[0],args[1])