import Crossfire,sys
Params=Crossfire.ScriptParameters()
whoami=Crossfire.WhoAmI()
Arch=None
if whoami.Name=="Magic Lever":
	Arch=whoami.Map.ObjectAt(49,2)
elif whoami.Name=="Buildable Lever":
	Arch=whoami.Map.ObjectAt(49,3)
StartXY=Params.split(":")[0].split(',')
StopXY=Params.split(":")[1].split(',')
Width=int(StopXY[0])-int(StartXY[0])
Height=int(StopXY[1])-int(StartXY[1])
Area=Width*Height
XRange=range(int(StartXY[0]),int(StopXY[0]))
YRange=range(int(StartXY[1]),int(StopXY[1]))
def GetObjectByName(Map,X,Y,Name):
		Object=Map.ObjectAt(X,Y)
		while Object!=None:
			if Object.Name==Name:
				return Object
			else:
				Object=Object.Above
		return Object
if Arch!=None:
	whoami.Value=int(whoami.Value==1)
	
	Arch.Quantity=Area+1

	
	#def MoveToBottom(object):
		#object.Say('s')
		#Dict={}
		#Counter=0
		#Item=object.Below
		#object.Say(str(Item))
		#while Item!=None:
			#Dict.update({str(Counter):Item})
			

			#Item=Item.Below
		#for i in list(Dict.values()):
			#i.Teleport(object.Map,object.X,object.Y)
			
	
	for i in XRange:
		for a in YRange:
			if whoami.Value==1:
				TmpArch=Arch.Split(1)
				TmpArch.Teleport(whoami.Map,i,a)
				#MoveToBottom(TmpArch)
			else:
				TmpArch=GetObjectByName(whoami.Map,i,a,Arch.Name)
				if TmpArch!=None:
					TmpArch.Remove()
				else:
					whoami.Say("Arch not found at "+str(i)+","+str(a))
					
			
else:
	for i in XRange:
		for a in YRange:
			t=whoami.Map.ObjectAt(i,a)
			
			while t!=None:
				if t.Floor==1:
					t.Name="Shop Floor"
				t=t.Above
			
