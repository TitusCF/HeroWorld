# -*- coding: utf-8 -*-
# setnpctoken.py
# This is one of the files that can be called by an npc_dialog,
# The following code runs when a dialog has a post rule of 'settoken'
# The syntax is ["setlocaltoken", "tokenname", "valuetosetto"]
# this can then be checked by a token condition that looks for the
# value of the token
# The token is kept in the NPC's data, and will be lost if the
# map containing the NPC resets.
## DIALOGCHECK
## MINARGS 2
## MAXARGS 2
## .*
## .*
## ENDDIALOGCHECK

self.setNPCStatus(args[0],args[1])