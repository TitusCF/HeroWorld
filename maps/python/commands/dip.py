import Crossfire

def find_fountain(pl):
    below = pl.Below
    while below is not None:
        if below.ArchName == "fountain":
            return below
        below = below.Below
    return None

def dip(pl):
    f = find_fountain(pl)
    if f is None:
        pl.Message("You must be at a fountain to dip an object into one.")
        return False

    ob = pl.MarkedItem
    if ob is None:
        pl.Message("Mark the item that you would like to dip.")
        return False

    name_before = ob.Name

    if ob.ArchName == "wbottle_empty":
        ob.Quantity -= 1
        w = Crossfire.CreateObjectByName("water")
        w.Identified = 1
        w.InsertInto(pl)
        pl.Message("You fill the %s with water from the %s." % (name_before, f.Name))
    else:
        pl.Message("You dip the %s into the %s. Nothing happens." % (name_before, f.Name))

dip(Crossfire.WhoAmI())
