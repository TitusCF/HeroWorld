import Crossfire

def do_repel():
	who.Say("%s, you're too violent, get out of here IMMEDIATELY!"%pl.Name)

	ex = who.Map.ObjectAt(exit_x, exit_y)
	while ex:
		if ex.Type == Crossfire.Type.EXIT:
			map = Crossfire.ReadyMap(ex.Slaying)
			if map:
				pl.Teleport(map, ex.HP, ex.SP)
				pl.Write('You feel a powerful force repel you!', Crossfire.MessageFlag.NDI_UNIQUE + Crossfire.MessageFlag.NDI_ORANGE)
				return
		ex = ex.Above


Crossfire.SetReturnValue(1)

who = Crossfire.WhoAmI()
who.HP = who.MaxHP

exit_x = 5
exit_y = 1

pl = Crossfire.WhoIsActivator()
while pl.Owner != None:
	pl = pl.Owner

if pl.Map == who.Map:
	do_repel()
