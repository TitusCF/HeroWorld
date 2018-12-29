
import Crossfire

import string
import random
import CFBank
import CFItemBroker
activator = Crossfire.WhoIsActivator()
activatorname = activator.Name
whoami = Crossfire.WhoAmI()
x = activator.X
y = activator.Y

message = whoami.Message
information=message
message1=string.split(message, "\n")
temp=string.split(message1[1], ":")
if temp[0]!="Amount":
	payee = message1[1]
	quantity=int(message1[2])
	currency=message1[3]

	message="Pay to the order of: " + payee + "\nAmount: " + str(quantity) + " "+currency + "\nSigned: " + activator.Name
	
whoami.Message = message

conversionfactorgold=10
conversionfactorplat=50
conversionfactorjade=5000
conversionfactoramber=50*10000








information=string.split(message, "\n")
payee=information[0]
amount=information[1]
signed=information[2]
payee=string.split(payee, ' ')
payee=payee[5]
amount=string.split(amount, ' ')
number=int(amount[1])
cointype=amount[2]
signed=string.split(signed, ' ')
signed=signed[1]


if cointype=="Silver":
	 conversionfactor=1
	 cointype1='silvercoin'
elif cointype == "Gold":
	conversionfactor=conversionfactorgold
	cointype1='goldcoin'
elif cointype == "Platinum":
	conversionfactor=conversionfactorplat
	cointype1='platinacoin'
elif cointype == "Jade":
	conversionfactor = conversionfactorjade
	cointype1='jadecoin'
elif cointype == "Amberium":
	conversionfactor = conversionfactoramber
	cointype1='amberiumcoin'
elif cointype == "Imperial":
	conversionfactor = 10000
	cointype1='imperial'


#if payee == "SHOP":
#		CFItemBroker.Item(whoami).subtract(1)
#		mailmap=Crossfire.ReadyMap('/planes/IPO_storage')
#		if mailmap:
#			newcheck = mailmap.ObjectAt(int (5), int (3))
#			newcheck.Name=activator.Name+"'s Check"
#			newcheck.NamePl=activator.Name+"'s Checks"
#			newcheck.Message=message
#			newcheck.Teleport(activator.Map,x,y)
#			newcheck.Value=int((number)*(conversionfactor))
#			activator.Say(str(int(number*conversionfactor)))
		
		
