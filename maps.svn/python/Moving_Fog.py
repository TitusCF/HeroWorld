import Crossfire,random,math
whoami=Crossfire.WhoAmI()
Params=Crossfire.ScriptParameters()

if Params=="GenerateFog":
	
	Fogs=int(whoami.Weight/10000)
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
		z.Speed+=0.05
		z.Weight=(0+random.randint(1,15+int(math.sqrt(FogsTmp))))*1
		Rand=random.randint(1,2+int(FogsTmp/10))
		z.Speed*=Rand
		z.Weight*=int(Rand/10)+1
		
		y=z.CreateObject("event_time")
		y.Name="Fog"
		y.Title="Python"
		y.Slaying="/python/Move_Fog.py"
		z.Speed/=3
		z.Weight=int(z.Weight)/3
