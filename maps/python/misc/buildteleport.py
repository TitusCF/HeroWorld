import Crossfire

me = Crossfire.WhoIsActivator()
building = Crossfire.WhoAmI()

entermap = Crossfire.ScriptParameters() # 1 is 'apply' event
enter_x = building.HP
enter_y = building.SP

me.Map.Print(entermap)

map = Crossfire.ReadyMap(entermap)
if map:
    me.Teleport(map, enter_x, enter_y)
else:
    print "There is no stoned map"
			
