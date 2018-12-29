import Crossfire
import CFGuilds
import CFItemBroker
import random
import string
import sys
import CFBank
import CFMail
import CFLog
activator=Crossfire.WhoIsActivator()
mymap=activator.Map

whoami=Crossfire.WhoAmI()
def GetForce(Target):
	while Target!=None:
		if Target.Name=="BigChest":
			return Target
		else:
			Target=Target.Below

if whoami.Name=="Big Chest":
	myPath=mymap.Path
	Target=activator.CheckInventory("BigChest")
	if Target !=None:
		Target.Quantity=0
	
	
	Card=activator.CreateObject("event_apply")
	Card.Name="BigChest"

	Card.Title=myPath
else:
	Target=activator.Inventory
	Target=GetForce(Target)
	if Target==None:
		whoami.Say("I'm sorry, I can't send you home.  It seems my attachment to the material plane has shifted.")
	else:
		Path=Target.Title
		Map=Crossfire.ReadyMap(Path)
		Target.Remove()

		activator.Teleport(Map, 1,9)
	
