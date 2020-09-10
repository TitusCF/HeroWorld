import Crossfire

me = Crossfire.WhoAmI()
ac = Crossfire.WhoIsActivator()

# Since this checks before equipping actually occurs, Applied will be 0 when we apply
if me.Applied == 0:
    ac.Write("You feel the "+me.Name+" bind to you.")
    me.Cursed = 1
    me.KnownCursed = 1
