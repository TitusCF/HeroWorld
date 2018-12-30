# -*- coding: utf-8 -*-
# takeitem.py
# This is one of the files that can be called by an npc_dialog, 
# The following code runs when a dialog has a post rule of 'takeitem'
# The syntax is ["takeitem", "itemtotake", "quantitytotake"]
# "quantitytotake" is optional, if it is missing, then 1 is assumed.
# if it is 0, then *all* instances of the item are taken.
# The player must have a sufficiant quantity of the item being taken
# This should normally be determined by doing an "item" check in the
# pre-block of the rule
# items are matched by item name, not arch name.
## DIALOGCHECK
## MINARGS 1
## MAXARGS 2
## .*
## \d+
## ENDDIALOGCHECK

itemname = args[0]
if len(args) == 2:
    quantity = args[1]
else:
    quantity = 1
Crossfire.Log(Crossfire.LogDebug, "CFDialog: trying to take: %s of item %s from character %s" %(quantity, itemname, character.Name ))
if itemname == "money":
    paid = character.PayAmount(int(quantity))
    if paid == 0:
        Crossfire.Log(Crossfire.LogError, "Tried to make player %s pay more than they had" %(character.Name))
else:
    inv = character.CheckInventory(itemname)
    if inv:
        if quantity == 0:
            inv.Remove()
        else:
            status = CFItemBroker.Item(inv).subtract(int(quantity))
            if status == 0:
                Crossfire.Log(Crossfire.LogError, "Dialog script tried to remove more items than available from player %s" %(character.Name))
        # we might have been wearing an item that was taken.
        character.Fix()
    else:
        Crossfire.Log(Crossfire.LogError, "Dialog script tried to remove non-existant item from player %s" %(character.Name))
