import Crossfire,random,math,sys
import CFDataBase
CFDB=CFDataBase.CFDataBase("PShop")

Params=Crossfire.ScriptParameters()

def GetObjectByName(object, Name):
	while object.Name!=Name:
		object=object.Above
		if not object:
			return 0
	return object
def FindFridge(object):
	TRUE=False
	while object!=None:
		if object.Name=="Shop Floor":
			TRUE=True
			object=None
		else:
			object=object.Below
	return TRUE
def FindPuddle(object):
	a=object
	while object!=None:
		if object.Name.find("Puddle")>-1:
			
			return object
		else:
			object=object.Above
	object=a
	while object!=None:
		if object.Name.find("Puddle")>-1:
			
			return object
		else:
			object=object.Below
	return None
if Params=="Destroy":
     
	whoami=Crossfire.WhoAmI()
	a=whoami.Inventory
	while a !=None:
		a.Remove()
		a=whoami.Inventory
		
	
	
	b=whoami.CreateObject("icecube")
	b.Weight=0
	z=b.CreateObject("event_time")
	b.Speed=0.0010000000475*4
	if whoami.Map.Path.find("/world_")>-1:
		b.Speed=b.Speed*10
	z.Title="Python"
	z.Name="Puddle"
	z.Slaying="/python/pshop/Ice.py"
	z=b.CreateObject("event_destroy")
	z.Title="Python"
	z.Name="PuddleDeath"
	z.Slaying="/python/pshop/Ice.py"
	b.NamePl=str(1)
	b.Teleport(whoami.Map,whoami.X,whoami.Y)
	g=FindPuddle(b)
	if g!=None:
		b.Remove()
		b=g
	
	b.Weight+=whoami.Weight
	b.Name="Puddle"
	if b.Weight<2000:
		b.Face="rain1.111" 
	elif b.Weight<20000:
		b.Face="rain2.111"
	elif b.Weight<200000:
		b.Face="rain3.111"
	elif b.Weight<2000000:
		b.Face="rain4.111"
	else:
		b.Face="rain5.111"
	b.Pickable=0
elif Params=="FogDeath":
	whoami=Crossfire.WhoAmI()
	a=whoami.Inventory
	
	while a !=None:
		
		a.Remove()
		a=whoami.Inventory
elif Params=="Fog":
	whoami=Crossfire.WhoAmI()
	Rand=random.randint(0,8)
	if Rand!=0:
		whoami.Move(Rand)
	
	whoami.Weight-=1
	if whoami.Weight>=0:
		Crossfire.SetReturnValue(1)
	
	
elif Params=="Get":
	whoami=Crossfire.WhoAmI()
	activator=Crossfire.WhoIsActivator()
	
		
	Dict=CFDB.get("pshop")
	
	WL=str(whoami.WeightLimit)
	Me=Dict.get(WL)
	
	a=(Me[0],"PickedUp",Me[2])
	
	#Me=str(Me[0],(activator.X,activator.Y),Me[1])
	
	
	Dict.update({str(whoami.WeightLimit):(a[0],a[1],a[2])})
	CFDB.store("pshop",Dict)
		
		
	

elif Params=="Player":
	whoami=Crossfire.WhoAmI()
elif Params=="PuddleDeath":
	whoami=Crossfire.WhoAmI()
	
	Fogs=int(whoami.Weight/1000)
	Fogs=max(Fogs,1)
	a=whoami.Inventory
	while a !=None:
		a.Remove()
		a=whoami.Inventory
	FogsTmp=Fogs
	if whoami.Name=="fog":
		FogsTmp=whoami.Value
	if Fogs > 50:
		z=whoami.CreateObject("temp_fog")
		Z=z.CreateObject("event_destroy")
		Z.Name="PuddleDeath"
		Z.Title="Python"
		Z.Slaying="/python/pshop/Ice.py"
		z.Weight=(Fogs-50)*1000
		Fogs=50
		z.Speed*=2
		z.Value=FogsTmp
	
	for i in range(Fogs):
		
		z=whoami.CreateObject("temp_fog")
		z.Speed+=0.3
		z.Weight=(3+random.randint(1,10+int(math.sqrt(FogsTmp))))*3
		Rand=random.randint(1,2+int(FogsTmp/10))
		z.Speed*=Rand
		z.Weight*=int(Rand/10)+1
		
		y=z.CreateObject("event_time")
		y.Name="Fog"
		y.Title="Python"
		y.Slaying="/python/pshop/Ice.py"
		#y=z.CreateObject("event_destroy")
		#y.Name="FogDeath"
		#y.Title="Python"
		#y.Slaying="/python/pshop/Ice.py"
	
		
	
	
	
elif Params=="Puddle":
	
	whoami=Crossfire.WhoAmI()
	whoami.Value+=1
	
	
		
	
	
	#whoami.Speed+=(0.00010000000475/2)
	
	Mass=int(math.sqrt(whoami.Weight))
	IntMass=int(Mass/1000)
	for i in range(IntMass):
		z=whoami.CreateObject("temp_fog")
		y=z.CreateObject("event_time")
		y.Name="Fog"
		y.Title="Python"
		y.Slaying="/python/pshop/Ice.py"
		#y=z.CreateObject("event_destroy")
		#y.Name="FogDeath"
		#y.Title="Python"
		#y.Slaying="/python/pshop/Ice.py"
		z.Weight=(3+random.randint(1,int(IntMass)+10))
		z.Speed=5*z.Speed
		whoami.Drop(z)
	whoami.Weight-=Mass
	if whoami.Weight<=0:
		
		whoami.Remove()
	else:
		if whoami.Weight>20000:
			a=whoami.CreateObject('icecube')
			Mass=random.randint(1,int(whoami.Weight/2))
			whoami.Weight-=Mass
			
			a.Weight=Mass
			Direction=random.randint(1,8)
			whoami.Drop(a)
			tmp=0
			CTRL=0
			
			
			if whoami.Map.Path.find("/world/world_")>-1:
				tmp=0
				CTRL=0
				XY=(whoami.X,whoami.Y)
				
				while tmp==0:
					tmp=whoami.Move(Direction)
					Direction+=1
					if Direction==9:
						Direction=1
					CTRL+=1
					if CTRL==8:
						tmp=2
				a.Teleport(whoami.Map, whoami.X,whoami.Y)
				whoami.Teleport(whoami.Map,XY[0],XY[1])
					
			else:
				while tmp==0:
					tmp=a.Move(Direction)
					Direction+=1
					if Direction==9:
						Direction=1
					CTRL+=1
					if CTRL==8:
						tmp=1
			
			b=a.Map.ObjectAt(a.X,a.Y)
			b=FindPuddle(b)
			if b!=None:
				b.Weight+=a.Weight
				a.Remove()
				a=b
			
			else:
				a.Name=whoami.Name
			if a.Weight < 1600:
				a.Face="rain1.111"
			elif a.Weight < 3200:
				a.Face="rain2.111"
			elif a.Weight<6400:
				a.Face="rain3.111"
			elif a.Weight<12800:
				a.Face="rain4.111"
			else:
				a.Face="rain5.111"
			Z=False
			y=a.Inventory
			while y!=None:
				if y.Name.find("Puddle")>-1:
					Z=True
					y=None
				else:
					y=y.Below
			if Z==False:
				z=a.CreateObject("event_time")
				a.Speed=0.0010000000475*4
				
				z.Title="Python"
				z.Name="Puddle"
				z.Slaying="/python/pshop/Ice.py"
				z=a.CreateObject("event_destroy")
				z.Title="Python"
				z.Name="PuddleDeath"
				z.Slaying="/python/pshop/Ice.py"
				#a.CreateTimer(500,2)
				a.NamePl=str(1)
			a.Pickable=0
			a.Speed=(0.0010000000475*4)
		if whoami.Weight < 1600:
			whoami.Face="rain1.111"
		elif whoami.Weight < 3200:
			whoami.Face="rain2.111"
		elif whoami.Weight<6400:
			whoami.Face="rain3.111"
		elif whoami.Weight<12800:
			whoami.Face="rain4.111"
		else:
			whoami.Face="rain5.111"
		whoami.Speed=(0.0010000000475*4)+0.0010000000475*whoami.Value/(whoami.Weight/1000)
	
	
	#whoami.Speed=max(whoami.Speed,0.0010000000475*4)
	
	
elif Params=="Timer":
	whoami=Crossfire.WhoAmI()
	
	b=FindPuddle(whoami.Map.ObjectAt(whoami.X,whoami.Y))
	

	if b==None:
		b=whoami.CreateObject("icecube")
		b.Weight=0
		z=b.CreateObject("event_time")
		b.Speed=0.0010000000475*4
		if whoami.Map.Path.find("/world_")>-1:
			b.Speed=10*b.Speed
		z.Title="Python"
		z.Name="Puddle"
		z.Slaying="/python/pshop/Ice.py"
		z=b.CreateObject("event_destroy")
		z.Title="Python"
		z.Name="PuddleDeath"
		z.Slaying="/python/pshop/Ice.py"
		#b.CreateTimer(500,2)
		b.NamePl=str(1)
		
		whoami.Drop(b)
	#b.Speed+=.00001
	b.Name="Puddle"
	Mass=int(100+math.sqrt(whoami.Weight))
	b.Weight+=max(Mass,50)
	whoami.Weight-=max(Mass,50)
	b.Pickable=0
	
	if b.Weight<2000:
		b.Face="rain1.111" 
	elif b.Weight<4000:
		b.Face="rain2.111"
	elif b.Weight<8000:
		b.Face="rain3.111"
	elif b.Weight<16000:
		b.Face="rain4.111"
	else:
		b.Face="rain5.111"
	if whoami.Weight<=0:
		whoami.Quantity=0
	whoami.Speed+=0.00010000000475
else:
	whoami=Crossfire.WhoAmI()
	
	activator=Crossfire.WhoIsActivator()
	mymap=activator.Map
	


	
	t=mymap.ObjectAt(activator.X, activator.Y)
	
	if (FindFridge(t)):
		whoami.Speed=0
		
		
			
		Dict=CFDB.get("pshop")
		
		WL=str(whoami.WeightLimit)
		Me=Dict.get(WL)
		
		a=(Me[0],(activator.X,activator.Y),Me[2])
		
		#Me=str(Me[0],(activator.X,activator.Y),Me[1])
		
		
		Dict.update({str(whoami.WeightLimit):(a[0],a[1],a[2])})
		CFDB.store("pshop",Dict)
		
		Crossfire.SetReturnValue(0)

	else:
		whoami.Speed=0.0010000000475*4
		if activator.Map.Path.find("/world_")>-1:

			#whoami.CreateTimer(1,2)
			
			whoami.Speed=whoami.Speed*10

		
		Crossfire.SetReturnValue(0)
		


