# -*- coding: utf-8 -*-
# archininventory.py
# This is one of the files that can be called by an npc_dialog,
# The following code runs when a dialog has a pre rule of 'archininventory'
# The syntax is ["archininventory", "arch"]
# To deliver a True verdict, the player must have in his inventory as least one
# item of the specified archetype.
# This is useful to check for skills, or spells, the player knows.
#
## DIALOGCHECK
## MINARGS 1
## MAXARGS 1
## .*
## ENDDIALOGCHECK

arch = args[0]
if character.CheckArchInventory(arch) == None:
    verdict = False
