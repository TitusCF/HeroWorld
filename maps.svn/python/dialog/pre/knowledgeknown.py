# -*- coding: utf-8 -*-
#knowledgeknown.py
# This is one of the files that can be called by an npc_dialog,
# The following code runs when a dialog has a pre rule of 'knowledgeknown'
# The syntax is ["knowledgeknown", "knowledge"]
# To deliver a True verdict, the player must have the knowledge represented
# by "knowledge" (specific format depending on the type of knowledge).
#
## DIALOGCHECK
## MINARGS 1
## MAXARGS 1
## .*
## ENDDIALOGCHECK

knowledge = args[0]
if not character.KnowledgeKnown(knowledge):
    verdict = False
