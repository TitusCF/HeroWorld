import random

import Crossfire

def check_type(item):
    return item.Type == Crossfire.Type.WAND

def cursed():
    return Crossfire.WhoAmI().Cursed or Crossfire.WhoAmI().Damned

def improve(item, player):
    if not cursed():
        player.Message("The %s glows softly with a blue aura." % item.Name)
        item.Level += 1
    else:
        player.Message("The %s glows with a dark black aura." % item.Name)
        item.Level += -1

def charge(item, player):
    player.Message("The %s glows for a moment, then fades." % item.Name)
    if not cursed():
        item.Food += random.randint(4, 6)
    else:
        item.Food -= 1

def apply():
    player = Crossfire.WhoIsActivator()
    if player.Type != Crossfire.Type.PLAYER:
        return
    item = player.MarkedItem
    if item == None:
        player.Message("You must mark an item first!")
        return
    if not check_type(item):
        player.Message("Nothing happens.")
        return
    if "charge" in Crossfire.ScriptParameters():
        charge(item, player)
    else:
        improve(item, player)
    scroll = Crossfire.WhoAmI()
    scroll.Identified = True
    player.Message("The %s crumbles to dust." % scroll.Name)
    scroll.Remove()

apply()
Crossfire.SetReturnValue(1)
