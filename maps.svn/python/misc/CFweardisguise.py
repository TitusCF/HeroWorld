#CFweardisguise.py
# A little script to insert an informational force into the player inventory
# if a article is applied and remove the force if it is unapplied.
# For example if you put on a priest robe it will insert the option value into
# a force slaying field which can be checked against on a map.
#
# This script is meant for items that can be worn or carried really
# I can't say how it will react if you hook it to other types of objects.

import Crossfire

activator=Crossfire.WhoIsActivator()
activatorname=activator.Name
whoami=Crossfire.WhoAmI()

option=Crossfire.ScriptParameters() # 1 is apply event

if option:
    inv = activator.CheckInventory(option) #Remove any previous disguise
    if inv:
        inv.Remove()
        #Crossfire.Log(Crossfire.LogDebug, "removing tag")

    if not whoami.Applied: #is the object is being applied
        tag = activator.CreateInvisibleObjectInside(option)
        tag.Name=option
        #Crossfire.Log(Crossfire.LogDebug, "adding tag")
