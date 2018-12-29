# -*- coding: utf-8 -*-
# givecontents.py
# This is one of the files that can be called by an npc_dialog, 
# The following code runs when a dialog has a post rule of 'givecontents'
# The syntax is ["givecontents", "nameofcontainer"]
# The NPC must have a container with the name "nameofcontainer", either in their 
# inventory or in their NPC_Gift_Box if they have one. 
# If a suitable container exists, then a complete copy of the entire contents 
# of that container is given to the player. The container itself is *not* given 
# to the player 
# containers are matched by item name, not arch name.

## DIALOGCHECK
## MINARGS 1
## MAXARGS 1
## .*
## ENDDIALOGCHECK

contents=True
# commongive.py does all of the heavy lifting here.
exec(open(os.path.join(Crossfire.DataDirectory(), Crossfire.MapDirectory(), 'python/dialog/commongive.py')).read())