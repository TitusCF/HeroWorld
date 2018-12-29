import Crossfire,datetime,CFDataBase
CFDB=CFDataBase.CFDataBase("PShop")
def GetObjectByWeightLimit(object, WeightLimit):
	while object.WeightLimit!=WeightLimit:
		object=object.Above
		if not object:
			return 0
	return object
def GetObjectByName(object, Name):
	while object.Name!=Name:
		object=object.Above
		if not object:
			return 0
	return object
	
def find_player(object):
    while (object.Type != 1) : #1 is type 'Player'
        object = object.Above
        if not object:
            return 0
    return object
def GetObjectAt(Map,X,Y,Name):
		Object=Map.ObjectAt(X,Y)
		while Object!=None:
			if Object.Name==Name:
				return Object
			else:
				Object=Object.Above
		return Object
def Expire():
	global Owner
	
	Inventory=mymap.ObjectAt(43,2)
	MailMap=Crossfire.ReadyMap("/planes/IPO_storage")
	while Inventory!=None:
		if Inventory.Name!="Vault":
			package=whoami.CreateObject("package")
			package.Name="IPO-package F: Your-Private-Shop T: "+Owner
			Inventory.InsertInto(package)
			package.Teleport(MailMap,2,2)
			
		Inventory=Inventory.Above
	Inventory=mymap.ObjectAt(43,2)
	while Inventory!=None:
		if Inventory.Name!="Vault":
			package=whoami.CreateObject("package")
			package.Name="IPO-package F: Your-Private-Shop T: "+Owner
			Inventory.InsertInto(package)
			package.Teleport(MailMap,2,2)
			
		Inventory=Inventory.Below
	
	Dict=CFDB.get("pshop")
	
	for i in Dict:
		whoami.Say(i)
		This=Dict.get(i)
		whoami.Say(str(This))
		if This[1]!="PickedUp":
			That=mymap.ObjectAt(This[1][0],This[1][1])
			whoami.Say(str(That))
			if That!=None:
				That=GetObjectByWeightLimit(That,int(i))
				whoami.Say(str(That))
				if That!=0:
					That.Teleport(whoami.Map,37,0)
	
	CFDB.store('pshop',{})
	Owner="Unowned"		
	Chest=mymap.ObjectAt(30,8)
	Chest=GetObjectByName(Chest, "Rent Box")
	if Chest!=0:
		Chest.Teleport(mymap, 15,10)
	for i in range(0,34):
		for a in range(0,35):
			b=GetObjectAt(whoami.Map,i,a,'NoBuild')
			b.Remove
			b=GetObjectAt(whoami.Map,i,a,'NoSpell')
			b.Remove()
	
	GetObjectAt(whoami.Map,30,5,"Brazier material").Remove()
	GetObjectAt(whoami.Map,30,6,"Firepot material").Remove()
	GetObjectAt(whoami.Map,30,7,"Bright Firepot Material").Remove()
	GetObjectAt(whoami.Map,29,8,"Red CWall material").Remove()
	whoami.Map.ObjectAt(49,5).Teleport(whoami.Map,30,5)
	whoami.Map.ObjectAt(49,4).Teleport(whoami.Map,30,6)
	whoami.Map.ObjectAt(49,6).Teleport(whoami.Map,30,7)
	whoami.Map.ObjectAt(49,7).Teleport(whoami.Map,29,8)
whoami=Crossfire.WhoAmI()

	

activator=Crossfire.WhoIsActivator()
activatorname=activator.Name
mymap=activator.Map

Variables=GetObjectByName(mymap.ObjectAt(49,0),"Variables")


VariableList= Variables.Message.split('\n')
Owner=VariableList[0]
Date=VariableList[1]
Days=VariableList[2]

Owner=Owner.split(": ")[1]
Date=Date.split(": ")[1]

Days=int(Days.split(": ")[1])

Year, Month, Day=Date.split("-")
LastDate=datetime.datetime(int(Year),int(Month),int(Day))
Today=datetime.date(1,2,3).today()
Today=datetime.datetime(Today.year, Today.month, Today.day)
DaysPast=(Today-LastDate).days
Days-=DaysPast

if Days<=0:
	global Owner
	
	
	Expire()

Variables.Message="Owner: "+Owner+"\nDate: "+str(Today.year)+"-"+str(Today.month)+"-"+str(Today.day)+"\nDays: "+str(Days)


if activatorname==Owner:
	message=Crossfire.WhatIsMessage()
	if (message.upper().find("ENTER")>-1) or (message.upper().find("YES")>-1):
		X=activator.X
		Y=activator.Y
		activator.Teleport(mymap, 37,3)
		activator.Teleport(mymap, X, Y)
		whoami.Say("Greetings sire, you have "+str(Days)+" days left.")
	else:
		whoami.Say("Greetings sire, would you like entry?")
else:
	
	if Owner!="Unowned":
		whoami.Say("You are not alowed beyond this point.  The rent will by up in "+str(Days)+" and you may rent it yourself then.  If the current owner doesn't pay, that is.")
	else:
		whoami.Say("You are currently not alowed beyond this point.  The rent is past due.  If you wish to proceed, place a platinum coin in the deposit box.")