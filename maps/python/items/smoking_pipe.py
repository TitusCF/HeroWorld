import Crossfire

# archetype that'll be smoked
smoke_what = 'pipeweed'
color = Crossfire.MessageFlag.NDI_BLUE

def smoke():
	if who.Type != Crossfire.Type.PLAYER:
		return
	what = who.Inventory
	while what:
		if what.ArchName == smoke_what:
			break
		what = what.Below
	if what == None:
		who.Write('You don\'t have anything to smoke.', color)
		return

	what.Quantity = what.Quantity - 1
	force = who.CreateObject('force_effect')
	force.Speed = 0.1
	force.Duration = 50
	force.Con = -2
	force.Dex = -2
	force.Applied = 1
	force.SetResist(Crossfire.AttackTypeNumber.FEAR, 100)
	who.ChangeAbil(force)

Crossfire.SetReturnValue(1)

me = Crossfire.WhoAmI()
who = Crossfire.WhoIsActivator()

smoke()
