import Crossfire,random,math
Params=Crossfire.ScriptParameters()
if Params=="Fog":
	whoami=Crossfire.WhoAmI()
	Rand=random.randint(0,8)
	if Rand!=0:
		whoami.Move(Rand)
	whoami.Speed*=1.5
	whoami.Weight-=1
	if whoami.Weight>=0:
		Crossfire.SetReturnValue(1)
