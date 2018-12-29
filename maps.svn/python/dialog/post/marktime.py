# -*- coding: utf-8 -*-
# marktime.py
# This is one of the files that can be called by an npc_dialog, 
# The following code runs when a dialog has a post rule of 'marktime'
# The syntax is ["marktime", "nameofmarker"]
# this can then be checked by an age condition that looks for the age 
# of "nameofmarker"

## DIALOGCHECK
## MINARGS 1
## MAXARGS 1
## .*
## ENDDIALOGCHECK

markername = args[0]
timestamp = map(str, (Crossfire.GetTime())[:5])
self.setStatus(markername, "-".join(timestamp))
