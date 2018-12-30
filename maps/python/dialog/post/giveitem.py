# -*- coding: utf-8 -*-
# giveitem.py
# This is one of the files that can be called by an npc_dialog, 
# The following code runs when a dialog has a post rule of 'giveitem'
# The syntax is ["giveitem", "itemtogive", "quantitytogive"]
# "quantitytogive" is optional, if it is missing, then 1 is assumed.
# The NPC must have at least one of the item being given, either in their 
# inventory or in their NPC_Gift_Box if they have one.
# items are matched by item name, not arch name.

## DIALOGCHECK
## MINARGS 1
## MAXARGS 2
## .*
## \d
## ENDDIALOGCHECK

contents=False
# commongive.py does all of the heavy lifting here.
exec(open(os.path.join(Crossfire.DataDirectory(), Crossfire.MapDirectory(), 'python/dialog/commongive.py')).read())