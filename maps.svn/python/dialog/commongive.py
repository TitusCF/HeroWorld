# -*- coding: utf-8 -*-
# commongive.py
# This is a common block of 'give' code to handle give item and give contents.

itemname = args[0]
if len(args) == 2:
    quantity = int(args[1])
else:
    quantity = 1
if itemname == "money":
    # we can't guarentee that the player has any particular type of coin already
    # so create the object first, then add 1 less than the total.
    if quantity >= 50:
        id = character.CreateObject('platinum coin')
        CFItemBroker.Item(id).add(int(quantity/50))
    if quantity % 50 > 0:
        id = character.CreateObject('gold coin')
        CFItemBroker.Item(id).add(int((quantity % 50)/10))
    if quantity % 50 > 0:
        id = character.CreateObject('silver coin')
        CFItemBroker.Item(id).add(int(quantity % 10))
else:
    # what we will do, is increase the number of items the NPC is holding, then
    # split the stack into the players inventory.
    # first we will check if there is an NPC_Gift_Box, and look in there.
    lookin = speaker.CheckInventory("NPC_Gift_Box")
    if lookin:
        inv = lookin.CheckInventory(itemname)
        if not inv:
            # ok, the NPC has no 'Gift Box', we'll check the other items.
            inv = speaker.CheckInventory(itemname)
    else:
        inv = speaker.CheckInventory(itemname)

    if inv:
        if contents:
            nextob=inv.Inventory
            while nextob:
                # when giving the contents of a container, always give the 
                # number of items in the container, not the quantity number.
                quantity = nextob.Quantity
                if quantity == 0:
                    # if quantity is 0, then we need to set it to one, otherwise bad things happen.
                    nextob.Quantity = 1
                    quantity = 1
                newob = nextob.Clone(0)
                newob.Quantity = quantity
                newob.InsertInto(character)
                nextob=nextob.Below
        else:
            if quantity == 0:
                nextob.Quantity = 2
                quantity = 1
            else:
                CFItemBroker.Item(inv).add(quantity+1)
            newob = inv.Split(quantity)

            newob.InsertInto(character)
    else:
        # ok, we didn't find any 
        Crossfire.Log(Crossfire.LogError, "Dialog script tried to give a non-existant item to a player")