

import Crossfire,sys,datetime
whoami=Crossfire.WhoAmI()
activator=Crossfire.WhoIsActivator()
import CFDataBase,CFBank
#sys.stderr=open("/tmp/output.log",'a')
sys.stderr=sys.stdout=open("/tmp/output.log2",'a')
#for i in dir(CFDataBase):
	#print i
#print CFDataBase.__file__

CFDB=CFDataBase.CFDataBase("PShop")
PicDB=CFDataBase.CFDataBase("PicDB")
PicDict=PicDB.get("Dict")
bank=CFBank.CFBank("ImperialBank_DB")
pshop=whoami.Map.Path.replace("/","_")


print CFDB.get(pshop)
print pshop
sys.stdout.flush()



activator=Crossfire.WhoIsActivator()

	
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
	
	Dict=CFDB.get(pshop)
	
	for i in Dict:
		This=Dict.get(i)
		if This[1]!="PickedUp":
			That=mymap.ObjectAt(This[1][0],This[1][1])
			if That!=None:
				That=GetObjectByWeightLimit(That,int(i))
				if That!=0:
					That.Teleport(whoami.Map,37,0)
	
	CFDB.store(pshop,{})
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
def GetObjectByName(object, Name):
	while object.Name!=Name:
		object=object.Above
		if not object:
			return 0
	return object
def GetObjectByWeightLimit(object, WeightLimit):
	while object.WeightLimit!=WeightLimit:
		object=object.Above
		if not object:
			return 0
	return object
def GetObjectByUID(object, UID):
	while object.Name.split()[0]!=UID:
		object=object.Above
		if not object:
			return 0
	return object
def GetInvCount(object):
	Inv=object.Inventory
	Counter=-1
	while Inv!=None:
		Counter+=1
		Inv=Inv.Below
	return Counter
mymap=whoami.Map
CoinTypes={"SILVER":1,"GOLD":10,"PLATINUM":50,"JADE":5000,"AMBER":500000,"IMPERIAL":10000}
Params=Crossfire.ScriptParameters()
if whoami.Name.find("Store")>-1:
	
	
	InvCount=GetInvCount(whoami)
	if InvCount==0:
		whoami.Say("Useage: Put an item in me and name a price.  For details, ask for 'help'.")
	
	else:
		Message=Crossfire.WhatIsMessage()
		Message=Message.split()
		Value=0
		for i in range(len(Message)):
			CoinType=CoinTypes.get(Message[i].upper())
			if CoinType!=None:
				Quantity=int(Message[i-1])
				Value+=Quantity*CoinType
		Package=whoami.CreateObject("dust_generic")
		Package.Face="package.111"
		Package.Teleport(whoami.Map,43,2)
		
		for i in range(InvCount):
			Item=whoami.Inventory
			Item.InsertInto(Package)
		
		Package.Name=str(whoami.Value)+" "+str(Value)
		Package.Speed=whoami.Value
		Package.WeightLimit=0
		z=Package.CreateObject("event_pickup")
		z.Name="Pickup"
		z.Title="Python"
		z.Slaying="/python/pshop/pshop.py"
		
		
		
		
		#STRING=STRING.split("Items: ")[1]
		
		if Item.Name==Crossfire.ScriptParameters(): 
			Item=Item.Below
		
		
		
		GlassReplica=whoami.CreateObject("icecube")
		GlassReplica.WeightLimit=whoami.Value
		GlassReplica.Weight=max(Item.Weight*Item.Quantity,1)
		GlassReplica.Quantity=1
		t=GlassReplica.CreateObject("event_drop")
		t.Name="Drop"
		t.Value=whoami.Value
		t.Title="Python"
		t.Slaying="/python/pshop/Ice.py"
		t=GlassReplica.CreateObject("event_pickup")
		t.Name="Get"
		t.Title="Python"
		t.Slaying="/python/pshop/Ice.py"
		t=GlassReplica.CreateObject("event_destroy")
		t.Name="Destroy"
		t.Title="Python"
		t.Slaying="/python/pshop/Ice.py"
		t=GlassReplica.CreateObject("event_time")
		t.Name="Timer"
		t.Title="Python"
		t.Slaying="/python/pshop/Ice.py"
		GlassReplica.Speed=0.0010000000475
		sys.stderr=open('/tmp/output.log', 'w')
		
		GlassReplica.Face=(str(Item.Face))
		#GlassReplica.Material=(96,'ice')
		
		GlassReplica.Name=str(Item.Quantity)+" "+Item.Name+" Price: "+str(Value)
		GlassReplica.NamePl="0"
		
		
		Message="Name: "+str(Item.Name)
		Message+="\nTitle: "+str(Item.Title)
		#Message+="\nMaterial: "+str(Item.Material.get("MaterialName"))
		Message+="\nQuantity: "+str(Item.Quantity)
		if Item.Cursed==1:
			Message+="\nCursed: True"
		Message+="\nWeight: "+str(Item.Weight)
		
		if Item.Dam!=0:
			Message+="\nDam: "+str(Item.Dam)
		if Item.AC!=0:
			Message+="\nAC: "+str(Item.AC)
		if Item.WC!=0:
			Message+="\nWC: "+str(Item.WC)
		if Item.Wis!=0:
			Message+="\nWis: "+str(Item.Wis)
		if Item.Str!=0:
			Message+="\nStr: "+str(Item.Str)
		
		if Item.Pow!=0:
			Message+="\nPow: "+str(Item.Pow)
		if Item.Int!=0:
			Message+="\nInt: "+str(Item.Int)
		if Item.Dex!=0:
			Message+="\nDex: "+str(Item.Dex)
		if Item.Con!=0:
			Message+="\nCon: "+str(Item.Con)
		if Item.Cha!=0:
			Message+="\nCha: "+str(Item.Cha)
		if Item.Value!=0:
			Message+="\nValue: "+str(Item.Value)
		GlassReplica.Message=Message
		Dict=CFDB.get(pshop)
		if Dict==0:
			Dict={}
		if Dict==None:
			Dict={}
		Dict.update({str(whoami.Value):(Value,"PickedUp",Message)})
		
		
		CFDB.store(pshop,Dict)

		whoami.Value+=1
elif whoami.Name.find("Rent Box")>-1:
	Value=0
	Inventory=whoami.Inventory
	
	while Inventory!=None:
		
		if Inventory.ArchName=="event_close":
			Inventory=Inventory.Above
		else:
			Value+=Inventory.Value*Inventory.Quantity

			Inventory.Teleport(whoami.Map,37,5)
			Inventory=whoami.Inventory
	Variables=whoami.Map.ObjectAt(49,0)
	Variables=GetObjectByName(Variables,"Variables")
	VariableList= Variables.Message.split('\n')
	Owner=VariableList[0].split(": ")[1]
	Date=VariableList[1].split(": ")[1]
	Days=int(VariableList[2].split(": ")[1])
	


	Year, Month, Day=Date.split("-")
	LastDate=datetime.datetime(int(Year),int(Month),int(Day))
	Today=datetime.date(1,2,3).today()
	Today=datetime.datetime(Today.year, Today.month, Today.day)
	DaysPast=(Today-LastDate).days
	Days-=DaysPast
	MaxDays=int(Params)
	if Owner!="Unowned":
		if Days<30:
			DT30=30-Days
			if DT30<=Value/50:
				Value-=DT30*50
				Days=30
			else:
				Days+=int(Value/50)
				Value-=int(Value/50)*50
		
			
			
		if Value>=100:
			if Days<60:
				DT60=60-Days
				if DT60<=Value/100:
					Value-=DT60*100
					Days=60
				else:
					Days+=int(Value/100)
					Value-=int(Value/100)*100
		
		if Value>=200:
			if Days<90:
				DT90=90-Days
				if DT90<=Value/200:
					Value-=DT90*200
					Days=90
				else:
					Days+=int(Value/200)
					Value-=int(Value/200)*200
		if Value>=400:
			if Days<120:
				DT120=120-Days
				if DT120<=Value/400:
					Value-=DT120*400
					Days=120
				else:
					Days+=int(Value/400)
					Value-=int(Value/400)*400
		if Value>=800:
			if Days<150:
				DT150=150-Days
				if DT150<=Value/800:
					Value-=DT150*800
					Days=150
				else:
					Days+=int(Value/800)
					Value-=int(Value/800)*800
		if Value>=1600:
			if Days<180:
				DT180=180-Days
				if DT90<=Value/1600:
					Value-=DT180*1600
					Days=180
				else:
					Days+=int(Value/1600)
					Value-=int(Value/1600)*1600
		if Value>=3200:
			if Days<210:
				DT210=210-Days
				if DT210<=Value/3200:
					Value-=DT210*3200
					Days=210
				else:
					Days+=int(Value/3200)
					Value-=int(Value/3200)*3200
		if Value>=6400:
			if Days<240:
				DT240=240-Days
				if DT240<=Value/6400:
					Value-=DT240*6400
					Days=240
				else:
					Days+=int(Value/6400)
					Value-=int(Value/6400)*6400
		if Value>=12800:
			if Days<270:
				DT270=270-Days
				if DT270<=Value/12800:
					Value-=DT270*12800
					Days=270
				else:
					Days+=int(Value/12800)
					Value-=int(Value/12800)*12800
		if Value>=25600:
			if Days<300:
				DT300=300-Days
				if DT300<=Value/25600:
					Value-=DT300*25600
					Days=300
				else:
					Days+=int(Value/25600)
					Value-=int(Value/25600)*25600
		
		if Value>=51200:
			if Days<330:
				DT330=330-Days
				if DT330<=Value/51200:
					Value-=DT330*51200
					Days=330
				else:
					Days+=int(Value/51200)
					Value-=int(Value/51200)*51200
		if Value>=102400:
			if Days<366:
				DT366=366-Days
				if DT366<=Value/102400:
					Value-=DT366*102400
					Days=366
				else:
					Days+=int(Value/102400)
					Value-=int(Value/102400)*102400
		tmp=whoami.CreateObject("silvercoin")
		tmp.Quantity=Value
		Days=min(Days,MaxDays)
	else:
		DT30=30
		Owner=activator.Name
		if DT30<=Value/50:
			Value-=DT30*50
			Days=30
		else:
			Days+=int(Value/50)
			Value-=int(Value/50)
		if Value>0:
			tmp=whoami.CreateObject("silvercoin")
			tmp.Quantity=Value
	if Days>=1:
		whoami.Teleport(whoami.Map,30,8)
	else:
		whoami.Teleport(whoami.Map,15,10)
	Variables.Message="Owner: "+Owner+"\nDate: "+str(Today.year)+"-"+str(Today.month)+"-"+str(Today.day)+"\nDays: "+str(Days)	
elif Params=="Trade":
	Message=Crossfire.WhatIsMessage().split()
	if Message[0].upper()=='TRADE':
		Name=''
		Title=''
		Quantity=0
		for i in range(len(Message)):
			
			if Message[i].upper()=="NAME":
				Name=Message[i+1]
			elif Message[i].upper()=='TITLE':
				Title=Message[i+1]
			elif Message[i].upper()=='QUANTITY':
				Quantity=int(Message[i+1])
elif Params=="Exchange":
	pass
elif Params=="InventorySay":
	
	Message=Crossfire.WhatIsMessage().split()
	if Message[0]=="Remove":
		
		CFDB.store(pshop,None)
	if Message[0].upper().find("DETAIL")>-1:
		Item=' '.join(Message[1:])
		
		Dict=CFDB.get(pshop)
		
		if Dict==0:
			Dict={}
		
		Ctrl=1
		for i in Dict:
			Str=Dict.get(str(i))
			Name=Str[2].split('\n')[0].split("Name: ")[1]
			
			if Item==Name:
				
				Message="\n"+str(Dict.get(i)[2])
				
				Message+=("\nAsking Price: "+str(Dict.get(i)[0])+"\nLocation: ")
				
				if Dict.get(i)[1]=="PickedUp":
					
					Message+="Not on store floor"
					
				elif Dict.get(i)[1][1]<=9:
					Message+="In employee only area."
				else:
					Message+=str(Dict.get(i)[1])
					Message+="\n(You are at ("+str(whoami.X)+","+str(whoami.Y)+")."
				whoami.Say(Message)
				Ctrl=0
		if Ctrl==1:
			
			
			Item=int(Message[1])
			
				
			Item=list(Dict)[Item-1]
				
			Item=Dict.get(str(Item))[2].split('\n')[0].split('Name: ')[1]
		
			for i in Dict:
				Str=Dict.get(str(i))
				Name=Str[2].split('\n')[0].split("Name: ")[1]
				
				if Item==Name:
					
					Message="\n"+str(Dict.get(i)[2])
					
					Message+=("\nAsking Price: "+str(Dict.get(i)[0])+"\nLocation: ")
					
					if Dict.get(i)[1]=="PickedUp":
						
						Message+="Not on store floor"
						
					elif Dict.get(i)[1][1]<=9:
						Message+="In employee only area."
					else:
						Message+=str(Dict.get(i)[1])
						Message+="\n(You are at ("+str(whoami.X)+","+str(whoami.Y)+")."
					whoami.Say(Message)
					Ctrl=0
elif Params=="AutoCheckout":
	Dict=CFDB.get(pshop)
	Inv=activator.Inventory
	Items=[]
	RealItems=[]
	MissingItems=[]
	Price=0
	
	while Inv!=None:
		
		if (Inv.Name.find("Price: ")>-1):
			
			Items=Items.__add__([Inv])
			
		Inv=Inv.Below
	for i in Items:
		Item=(activator.Map.ObjectAt(43,2))
		
		
		
		Item=GetObjectByUID(Item,str(i.WeightLimit))
		
		if Item!=0:
			
			Price+=int(Item.Name.split()[1])
			RealItems=RealItems.__add__([Item])
		else:
			
			whoami.Say("Item "+i.Name+" missing.")
			MissingItems=MissingItems.__add__([i])
	if activator.PayAmount(Price):
		for i in RealItems:
			i.InsertInto(activator)
			tmp=Dict.pop((str(i.Name.split()[0])))
		for i in Items:
			i.Teleport(activator.Map, 37,0)
			i.Speed+=(0.00010000000475*2)
			
		for i in MissingItems:
			i.Teleport(activator.Map,37,0)
			i.Speed+=(0.00010000000475*2)
			tmp=Dict.pop((str(i.WeightLimit)))
			
			
		CFDB.store(pshop,Dict)
		
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

		if Days<30:
			DT30=30-Days
			if DT30<=Price/50:
				Price-=DT30*50
				Days=30
			else:
				Days+=int(Price/50)
				Price-=int(Price/50)
		
		if Days<=0:
			global Owner
	
	
			Expire()

		Variables.Message="Owner: "+Owner+"\nDate: "+str(Today.year)+"-"+str(Today.month)+"-"+str(Today.day)+"\nDays: "+str(Days)	
		bank.deposit(Owner, int(Price/1.01))
		
	else:
		whoami.Say("You do not have enough cash, "+str(Price)+" silver needed.")
elif Params=="BankCheckout":
	Dict=CFDB.get(pshop)
	Inv=activator.Inventory
	Items=[]
	RealItems=[]
	MissingItems=[]
	Price=0
	
	while Inv!=None:
		if (Inv.Name.find("Price: ")>-1):
			
			Items=Items.__add__([Inv])
			
		Inv=Inv.Below
	
	for i in Items:
		Item=(activator.Map.ObjectAt(43,2))
		
		
		
		Item=GetObjectByUID(Item,str(i.WeightLimit))
		
		if Item!=0:
			
			Price+=int(Item.Name.split()[1])
			RealItems=RealItems.__add__([Item])
		else:
			
			whoami.Say("Item "+i.Name+" missing.")
			MissingItems=MissingItems.__add__([i])
	if bank.getbalance(activator.Name)>=Price:
		bank.withdraw(activator.Name, Price)
		for i in RealItems:
			i.InsertInto(activator)
			tmp=Dict.pop((str(i.Name.split()[0])))
		for i in Items:
			i.Teleport(activator.Map, 37,0)
			i.Speed+=(0.00010000000475*2)
			
		for i in MissingItems:
			i.Teleport(activator.Map,37,0)
			i.Speed+=(0.00010000000475*2)
			tmp=Dict.pop((str(i.WeightLimit)))
			
			
		CFDB.store(pshop,Dict)
		
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

		if Days<30:
			DT30=30-Days
			if DT30<=Price/50:
				Price-=DT30*50
				Days=30
			else:
				Days+=int(Price/50)
				Price-=int(Price/50)
		
		if Days<=0:
			global Owner
	
	
			Expire()

		Variables.Message="Owner: "+Owner+"\nDate: "+str(Today.year)+"-"+str(Today.month)+"-"+str(Today.day)+"\nDays: "+str(Days)	
		bank.deposit(Owner, int(Price/1.01))
		
	else:
		whoami.Say("You do not have enough funds in the bank. "+str(Price)+" needed.")
elif Params=="TrashOpen":
	Trash=whoami.Map.ObjectAt(37,0)
	while Trash!=None:
		Trash1=Trash.Above
		Trash.InsertInto(whoami)
		Trash=Trash1
elif Params=="TrashClose":
	Trash=whoami.Inventory
	while Trash!=None:
		if (Trash.Name=="TrashClose") or (Trash.Name=="TrashOpen"):
			Trash=Trash.Above
		else:
			Trash.Teleport(whoami.Map,37,0)
			
			if Trash.ArchName=="icecube":
				Trash.Speed+=(0.00010000000475*2)
			Trash=whoami.Inventory
			
else:
	whoami.Message="xyzzy"
	Dict=CFDB.get(pshop)
	if Dict==0:
		Dict={}
	whoami.Message=''
	for i in Dict:
		Str=Dict.get(i)[2].split('\n')[0]
		
		whoami.Message+=str(Str)
	
