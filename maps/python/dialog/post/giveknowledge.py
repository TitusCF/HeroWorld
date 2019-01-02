# -*- coding: utf-8 -*-
# giveknowledge.py
# This is one of the files that can be called by an npc_dialog,
# The following code runs when a dialog has a post rule of 'giveknowledge'
# The syntax is ["giveknowledge", "knowledge code"]
# The player will receive the specified knowledge, from its code.
# Note: knowledge code is a specific format, ask on #irc or on the mailing lists.

## DIALOGCHECK
## MINARGS 1
## MAXARGS 1
## .*
## ENDDIALOGCHECK

knowledge = args[0]
character.GiveKnowledge(knowledge)
