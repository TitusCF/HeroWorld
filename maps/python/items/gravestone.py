import Crossfire

def event():
    grave = Crossfire.WhoAmI()
    activator = Crossfire.WhoIsActivator()
    if not grave.Pickable:
        # already fixed, do nothing
        return
    if grave.Env is not None or grave.Below.ArchName != 'bosgrass15':
        activator.Message("Drop the %s on a Grave Plot and then apply." % grave.Name)
        return
    grave.Pickable = False
    grave.Unique = True
    reward = Crossfire.CreateObjectByName("minor_potion_restoration")
    name = reward.Name
    reward.InsertInto(activator)
    activator.Message("You fix the %s in place. You earned a %s." % (grave.Name, name))

event()
