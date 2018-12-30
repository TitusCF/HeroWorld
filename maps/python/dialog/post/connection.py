# -*- coding: utf-8 -*-
# connection.py
# This is one of the files that can be called by an npc_dialog, 
# The following code runs when a dialog has a post rule of 'connection'
# The syntax is ["connection", "numberofconnection"]
# Triggers the numbered connection on the local map in the same way as 
# if button with that connection value had been pressed.
## DIALOGCHECK
## MINARGS 1
## MAXARGS 1
## \d+
## ENDDIALOGCHECK

speaker.Map.TriggerConnected(int(condition[1]), 1, speaker)