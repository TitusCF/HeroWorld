import Crossfire

def check_type(item):
    return item.Type == Crossfire.Type.WAND

def improvement():
    if Crossfire.WhoAmI().Cursed or Crossfire.WhoAmI().Damned:
        return -1
    else:
        return 1

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
    n = improvement()
    if n > 0:
        player.Message("The %s glows softly with a blue aura." % item.Name)
    else:
        player.Message("The %s glows with a dark black aura." % item.Name)
    item.Level += n
    scroll = Crossfire.WhoAmI()
    scroll.Identified = True
    player.Message("The %s crumbles to dust." % scroll.Name)
    scroll.Remove()

apply()
Crossfire.SetReturnValue(1)
