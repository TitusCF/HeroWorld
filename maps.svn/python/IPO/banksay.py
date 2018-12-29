# Script for say event of Imperial Bank Tellers
#
# Copyright (C) 2002 Joris Bontje
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# The author can be reached via e-mail at jbontje@suespammers.org
#
# Updated to use new path functions in CFPython and broken and
# modified a bit by -Todd Mitchell


import Crossfire

import string
import random
import CFBank
import CFItemBroker
import CFMail
mail=CFMail.CFMail()
Skuds="Imperial-Bank-Of-Skud"+str("Imperial-Bank-Of-Skud".__hash__()) #account to put fees into.  Let's everyone see how much the bank is actually being used.
activator = Crossfire.WhoIsActivator()
activatorname = activator.Name
whoami = Crossfire.WhoAmI()
import sys
sys.stdout=sys.stderr=open('/tmp/checks.log','w')
x = activator.X
y = activator.Y
CoinTypes={"SILVER":1,"GOLD":10,"PLATINUM":50,"JADE":5000,"AMBERIUM":500000, "IMPERIAL NOTE":10000,"10 IMPERIAL NOTE":100000,"100 IMPERIAL NOTE":1000000}
ArchType={"SILVER":"silvercoin","GOLD":"goldcoin","PLATINUM":"platinacoin","JADE":"jadecoin","AMBERIUM":"ambercoin", "IMPERIAL NOTE":"imperial","10 IMPERIAL NOTE":"imperial10","100 IMPERIAL NOTE":"imperial100"}

conversionfactorgold=10
conversionfactorplat=50
conversionfactorjade=5000
conversionfactoramber=50*10000

#EASILY SETTABLE PARAMETERS
service_charge = 1    # service charges for transactions as a percent

bankdatabase = "ImperialBank_DB"

fees = (service_charge/100.0)+1
bank = CFBank.CFBank(bankdatabase)


thanks_message = [ \
	'Thank you for banking the Imperial Way.', \
	'Thank you for banking the Imperial Way.', \
	'Thank you, please come again.', \
	'Thank you, please come again.', \
	'Thank you for your patronage.', \
	'Thank you for your patronage.', \
	'Thank you, have a nice day.', \
	'Thank you, have a nice day.', \
	'Thank you. "Service" is our middle name.', \
	'Thank you. "Service" is our middle name.', \
	'Thank you. Hows about a big slobbery kiss?' ]

mymap=activator.Map
if whoami.Name.find("Deposit Box")>-1:
	
	ScriptParm=Crossfire.ScriptParameters()
	if ScriptParm=="Close":
		t=activator.CheckInventory("SkudCtrl")
		
		
		
		Total=float(t.Title)
		Total=long(Total)
		
		t.Quantity=0
		
		MyInv=whoami.Inventory
		Value=0
		while MyInv!=None:
			if (MyInv.Name!="Apply") and (MyInv.Name!="Close"):
				
				Value+=MyInv.Value*MyInv.Quantity
				MyInv1=MyInv.Below
				MyInv.Teleport(mymap, 15,3)
				MyInv=MyInv1
			else:
				MyInv=MyInv.Below
		
		bank.deposit(activatorname, Value)
		Difference=abs(Value-Total)
		
		
		Fee=Difference-(Difference/fees)
		bank.withdraw(activatorname, int(Fee))
		
		whoami.Say("A Service charge of "+str(int(Fee))+" silver coins has been charged on this transaction.")
		
		
		
	else:
		
		balance=bank.getbalance(activatorname)
		Total=balance
		
		if balance >= 1000000:
			
			t=whoami.CreateObject("imperial100")
			
			bank.withdraw(activatorname,1000000)
			balance=bank.getbalance(activatorname)
		if balance >= 1000000/2:
			t=whoami.CreateObject("ambercoin")
			bank.withdraw(activatorname,1000000/2)
			balance=bank.getbalance(activatorname)
		if balance >= 1000000/2:
			t=whoami.CreateObject("ambercoin")
			bank.withdraw(activatorname,1000000/2)
			balance=bank.getbalance(activatorname)
		
		if balance >= 100000:
			t=whoami.CreateObject("imperial10")
			bank.withdraw(activatorname,100000)
			balance=bank.getbalance(activatorname)
		if balance >= 100000:
			t=whoami.CreateObject("imperial10")
			bank.withdraw(activatorname,100000)
			balance=bank.getbalance(activatorname)
		if balance >= 100000:
			t=whoami.CreateObject("imperial10")
			bank.withdraw(activatorname,100000)
			balance=bank.getbalance(activatorname)
		if balance >= 100000:
			t=whoami.CreateObject("imperial10")
			bank.withdraw(activatorname,100000)
			balance=bank.getbalance(activatorname)
		if balance >= 100000:
			t=whoami.CreateObject("imperial10")
			bank.withdraw(activatorname,100000)
			balance=bank.getbalance(activatorname)
		
		if balance >= 10000:
			t=whoami.CreateObject("imperial")
			bank.withdraw(activatorname,10000)
			balance=bank.getbalance(activatorname)
		if balance >= 10000:
			t=whoami.CreateObject("imperial")
			bank.withdraw(activatorname,10000)
			balance=bank.getbalance(activatorname)
		if balance >= 10000:
			t=whoami.CreateObject("imperial")
			bank.withdraw(activatorname,10000)
			balance=bank.getbalance(activatorname)
		if balance >= 10000:
			t=whoami.CreateObject("imperial")
			bank.withdraw(activatorname,10000)
			balance=bank.getbalance(activatorname)
		if balance >= 10000:
			t=whoami.CreateObject("imperial")
			bank.withdraw(activatorname,10000)
			balance=bank.getbalance(activatorname)
		if balance >= 10000:
			t=whoami.CreateObject("imperial")
			bank.withdraw(activatorname,10000)
			balance=bank.getbalance(activatorname)
		if balance >= 10000:
			t=whoami.CreateObject("imperial")
			bank.withdraw(activatorname,10000)
			balance=bank.getbalance(activatorname)
		if balance >= 10000:
			t=whoami.CreateObject("imperial")
			bank.withdraw(activatorname,10000)
			balance=bank.getbalance(activatorname)
		if balance >= 10000:
			t=whoami.CreateObject("imperial")
			bank.withdraw(activatorname,10000)
			balance=bank.getbalance(activatorname)
		if balance >= 10000:
			t=whoami.CreateObject("imperial")
			bank.withdraw(activatorname,10000)
			balance=bank.getbalance(activatorname)
		if balance >= 10000/2:
			t=whoami.CreateObject("jadecoin")
			bank.withdraw(activatorname,10000/2)
			balance=bank.getbalance(activatorname)
		if balance >= 10000/2:
			t=whoami.CreateObject("jadecoin")
			bank.withdraw(activatorname,10000/2)
			balance=bank.getbalance(activatorname)
		if balance >= 5000:
			t=whoami.CreateObject("platinacoin")
			bank.withdraw(activatorname,5000)
			t.Quantity=100
			balance=bank.getbalance(activatorname)
		if balance >= 1000:
			t=whoami.CreateObject("goldcoin")
			bank.withdraw(activatorname,1000)
			t.Quantity=100
			balance=bank.getbalance(activatorname)
		if balance >= 1000:
			t=whoami.CreateObject("silvercoin")
			bank.withdraw(activatorname,1000)
			t.Quantity=1000
			balance=bank.getbalance(activatorname)
		balance=bank.getbalance(activatorname)
		Total=Total-balance
		t=activator.CreateObject("force")
		t.Name="SkudCtrl"
		t.Title=str(Total)
		
		
			#tnew=t.InserInto(whoami)
if (whoami.Name != "Deposit Box"):
	text = Crossfire.WhatIsMessage().split()
	if text[0] == 'help' or text[0] == 'yes':
		message ='You can:\n-deposit,-withdraw,-balance,-exchange, -cash, -transfer, -checks, -profits\
		\nAll transactions are in imperial notes\n(1 : 1000 gold coins). \
		\nA service charge of %d percent will be placed on all deposits.' \
		%(service_charge)
		if activator.DungeonMaster==1:
			message+="\nYou can also zero the balance for the bank of skud's profit with the zero-balance command."
	elif text[0]=="zero-balance":
		bank.withdraw(Skuds, bank.getbalance(Skuds))
		message="Balance zeroed."
	elif text[0]=="profits":
		message="The Imperial Bank of Skud has made "+str(bank.getbalance(Skuds)*0.029394968)+" pounds sterling in profit to date."

	elif text[0] == 'deposit':
		import sys
		sys.stderr=open('/tmp/output.log','w')
		if len(text)==2:
			if text[1]=='check':
				whoami.Say('x')
				inv=activator.CheckInventory('bankcard')
				whoami.Say('x')
				if inv:
					whoami.Say('y')
					name1=string.split(inv.Name, "'")
					payer=name1[0]
					information=inv.Message
					information = string.split(information, "\n")
					if (information[0] !="CANCELED"):
						
						payee = string.split(information[0], ' ')
			#   whoami.Say(str(payee))
						
						payee=payee[5]
						if payee != activator.Name:
							if payee.upper() != "BEARER":
								message = "This check is not made out to you!"
				
								mail.send(1, payee, "The-Imperial-Bank-of-Skud", 'Dear Sir or Madam:'+ "\n" + 'It has come to our attention that an attempt to cash a check made out to you by someone other than you has been made.  The check was made out by' + payer + '; and, the attempt to cash it was made by' + activator.Name + '.  We apologise for any trouble this may cause, and would like to inform you that we are always at your service.' + "\n \n" + 'Sincerly,'+ "\n"+'The Imperial Bank of Skud' )
								mail.send(1, payer, "The-Imperial-Bank-of-Skud", 'Dear Sir or Madam:' + "\n" 'It has come to our attention that an attempt to cash a check made out by you to '+ payee + ' has been made by ' +activator.Name + '.  We apologise for any trouble this may cause; and, we would like to inform you that we are always at your service.'+"\n"+ "\n"+'Sincerly,'+"\n"+' The Imperial Bank of Skud') 
							if payee.upper() == "BEARER":
								payee = activator.Name
						if payee == activator.Name:
								balanceavailable=bank.getbalance(payer)
								amount=string.split(information[1],' ')
								signer = string.split(information[2])
								signer = signer[1]
								if payer == signer:
									cointype=amount[1]
						#print amount
				#		whoami.Say(str(amount))
									cointypecoin=amount[2]
									amount=int(amount[1])
						#print cointype
						#print amount
			
					#	conversionfactor=1
					#	cointype1='silvercoin'
									if cointypecoin=="Silver":
										conversionfactor=1
										cointype1='silvercoin'
									elif cointypecoin == "'Gold'":
										conversionfactor=10
										cointype1='goldcoin'
									elif cointypecoin == "'Platinum'":
										conversionfactor=50
										cointype1='platinacoin'
									elif cointypecoin == "Jade":
										conversionfactor = 5000
										cointype1='jadecoin'

									elif cointypecoin == "'Amberium'":
										conversionfactor = 500000
										cointype1='amberiumcoin'
									elif cointypecoin == "'Imperial'":
										conversionfactor = 10000
										cointype1='imperial'
									cashonhand=balanceavailable
									quantity = (((amount)*conversionfactor))
									if cashonhand >= quantity:
										if amount <= 0:
											message = "I'm sorry, We do not accept checks with a zero balance."
										elif amount == 1:
											message = "Okay, %s %s transfered to your account." %(cointype, cointypecoin)
											inv.Message="CANCELED\n"+inv.Message
											inv.Name = inv.Name + " (CANCELED)"
											bank.withdraw(payer,quantity)
											bank.deposit(payee,quantity)
								
										else:
											message = "Okay, %s %s transfered to your account." %(str(amount), cointypecoin)
											inv.Message="CANCELED\n"+inv.Message
											inv.Name = inv.Name + " (CANCELED)"
											bank.withdraw(payer,quantity)
											bank.deposit(payee,quantity)
									else:
										message = "I'm sorry, but it appears that %s does not have enough money in his account to cover this transaction." %(payer)
								else:
									message = "The signature on this check is invalid."
									mail.send(1,payer,"The-Imperial-Bank-Of-Skud", "Dear Sir or Madam:\nIt has come to our attention that an attempt to cash an improperly signed check was made.  The check was made out to " + payee + "and the attempt was made by " + activator.Name+".  The value of the check was " + str(amount[1]) +" "+str(amount[2])+' '+str(amount[3])+'.  We apologise for any trouble this may cause and would like to inform you that we are always at your service.'+"\n"+'Sincerly,'+"\n"+'The Imperial Bank of Skud')
					else:
						message="This check has already been cashed!"
				else:
					message = "Come back when you have a check to cash."
					whoami.Say('z')
						
						
						
						
						
						
						
						
						
						
						
						
						
						
						
						
						
						
						
						
						
						
						
						
						
				
		if len(text) >= 3:
			amount = int(text[1])
			Type=''
			for i in text[2:]:
				Type+=i+" "
			Type=Type[:len(Type)-1]
			
			if Type[len(Type)-1]=='s':
				Type=Type[:len(Type)-1]
			
			try:
				
				exchange_rate=CoinTypes.get(Type.upper())
				if amount <= 0:
					message = 'Usage "deposit <amount> <cointype>\n or deposit <issuer>\'s check"'
				
				elif activator.PayAmount(int(amount*exchange_rate)):
					bank.deposit(activatorname, int((amount*exchange_rate)/fees))
					bank.deposit(Skuds, int((amount*exchange_rate)-((amount*exchange_rate)/fees)))
					
					message = '%d %s received, %d %s deposited to bank account. %s' \
					%((amount),Type, int(amount/fees),Type, random.choice(thanks_message))
				else:
					message = 'You would need %d gold'%((amount*(exchange_rate/10)))
			except:
				message="Valid CoinTypes are Silver, Gold, Platinum, Jade, Amberium, Imperial Note, 10 Imperial Note, 100 Imperial Note.  \nPlease by sure to specify one of these types."
		else:
			#message = 'Usage "deposit <amount> <cointype>\n or deposit <issuer>\'s check"'
			pass
 
	elif text[0] == 'withdraw':
		if len(text) >=3:
			amount = int(text[1])
			Type=''
			for i in text[2:]:
				Type+=i+" "
			Type=Type[:len(Type)-1]
			
			if Type[len(Type)-1]=='s':
				Type=Type[:len(Type)-1]
			try:
				
				exchange_rate=int(CoinTypes.get(Type.upper()))
				if amount <= 0:
					message = 'Usage "withdraw <amount> <cointype>"'
				
				
				elif bank.withdraw(activatorname, amount*exchange_rate):
					
					message = '%d %s withdrawn from bank account. %s' \
					%(amount,Type, random.choice(thanks_message))
					
					
					id = activator.Map.CreateObject(ArchType.get(Type.upper()), x, y)
					CFItemBroker.Item(id).add(amount)
					activator.Take(id)
				else:
					message = 'Not enough silver on your account'
			except:
				
				message="Valid CoinTypes are Silver, Gold, Platinum, Jade, Amberium, Imperial Note, 10 Imperial Note, 100 Imperial Note.  \nPlease by sure to specify one of these types."
				
		else:
			message = 'Usage "withdraw <amount in imperials>"'


	elif text[0] == 'exchange':
		if len(text) == 2:
			amount = int(text[1])
			if amount <= 0:
				message = 'Usage "exchange <amount>" (imperials to platinum coins)'
			elif amount > 10000:
				message = 'Sorry, we do not exchange more than 10000 imperials all at once.'
			else:
				inv = activator.CheckInventory('imperial')
				if inv:
					pay = CFItemBroker.Item(inv).subtract(amount)
					if pay:
						exchange_rate=10000

						# Drop the coins on the floor, then try
						# to pick them up. This effectively
						# prevents the player from carrying too
						# many coins.
						id = activator.Map.CreateObject('platinum coin', x, y)
						CFItemBroker.Item(id).add(int(amount*(exchange_rate/50)))
						activator.Take(id)

						message = random.choice(thanks_message)
					else:
						message = 'Sorry, you do not have %d imperials'%(amount)
				else:
					message = 'Sorry, you do not have any imperials'
		else:
			message = 'Usage "exchange <amount>" (imperials to platinum coins)'
	

	elif text[0] == 'balance':
		if len(text)>=2:
			Type=''
			
			for i in text[1:]:
				Type+=i+" "
			Type=Type[:len(Type)-1]
			if Type[len(Type)-1]=='s':
				Type=Type[:len(Type)-1]
			exchange_rate=CoinTypes.get(Type.upper())
			balance=bank.getbalance(activatorname)/exchange_rate
			message= "Amount in bank: "
			if int(balance)==1:
				message+="1 "+Type+'.'
			elif int(balance)>1:
				message+=str(int(balance))+" "+Type
				
				if Type.find("Note")==-1:
					message+=" Coins."
				
				
			else:
				message="Your balance is less than 1 "+Type+'.'
		else:
			balance = bank.getbalance(activatorname)
			if balance == 1:
				message = 'Amount in bank: 1 silver coin'
			elif balance:
				message = 'Amount in bank: %d pounds sterling or %d silver coins.'%(balance*0.029394968, balance)
			else:
				message = 'Sorry, you have no balance.'
	elif text[0] == 'checks':
	

 


		balance=bank.getbalance(activatorname)
		if balance >=100:
			bank.withdraw(activatorname, 100)

			mailmap=Crossfire.ReadyMap('/planes/IPO_storage')
			
			if mailmap:
				pack=activator.Map.CreateObject('package', 1, 1)
				pack.Name = 'IPO-package F: The-Imperial-Bank-of-Skud T: ' + activator.Name
				pack.Teleport(mailmap, 2, 2)
				cheque = mailmap.ObjectAt(int (5), int (0))
				message = "Your checks will be mailed to you shortly.  Thank you for your patronage."
				cheque.Name = str(activator.Name + "'s Checkbook")
				cheques=cheque.CheckArchInventory("bankcard")
				cheques.Name = str(activator.Name + "'s Check")
				cheques.NamePl = str(activator.Name + "'s Checks")
				chequenew = cheque.InsertInto(pack)


			
			
		else:
			message = "You do not have sufficient funds in your bank account."


	elif text[0] == 'cash':
		inv=activator.CheckInventory('bankcard')
	 
		if inv:
			name1=string.split(inv.Name, "'")
			payer=name1[0]
			information=inv.Message
			information = string.split(information, "\n")
			if (information[0] !="CANCELED"):
				payee = string.split(information[0], ' ')
	#   whoami.Say(str(payee))
				
				payee=payee[5]
				if payee != activator.Name:
					if payee.upper() != "BEARER":
						message = "This check is not made out to you!"
		
						mail.send(1, payee, "The-Imperial-Bank-of-Skud", 'Dear Sir or Madam:'+ "\n" + 'It has come to our attention that an attempt to cash a check made out to you by someone other than you has been made.  The check was made out by' + payer + '; and, the attempt to cash it was made by' + activator.Name + '.  We apologise for any trouble this may cause, and would like to inform you that we are always at your service.' + "\n \n" + 'Sincerly,'+ "\n"+'The Imperial Bank of Skud' )
						mail.send(1, payer, "The-Imperial-Bank-of-Skud", 'Dear Sir or Madam:' + "\n" 'It has come to our attention that an attempt to cash a check made out by you to '+ payee + ' has been made by ' +activator.Name + '.  We apologise for any trouble this may cause; and, we would like to inform you that we are always at your service.'+"\n"+ "\n"+'Sincerly,'+"\n"+' The Imperial Bank of Skud') 
					if payee.upper() == "BEARER":
						payee = activator.Name
				if payee == activator.Name:
						balanceavailable=bank.getbalance(payer)
						amount=string.split(information[1],' ')
						signer = string.split(information[2])
						signer = signer[1]
						if payer == signer:
							cointype=amount[1]
				#print amount
		#		whoami.Say(str(amount))
							cointypecoin=amount[2]
							amount=int(amount[1])
				#print cointype
				#print amount
	
			#	conversionfactor=1
			#	cointype1='silvercoin'
							if cointypecoin=="Silver":
								conversionfactor=1
								cointype1='silvercoin'
							elif cointypecoin == "'Gold'":
								conversionfactor=10
								cointype1='goldcoin'
							elif cointypecoin == "'Platinum'":
								conversionfactor=50
								cointype1='platinacoin'
							elif cointypecoin == "Jade":
								conversionfactor = 5000
								cointype1='jadecoin'

							elif cointypecoin == "'Amberium'":
								conversionfactor = 500000
								cointype1='amberiumcoin'
							elif cointypecoin == "'Imperial'":
								conversionfactor = 10000
								cointype1='imperial'
							cashonhand=balanceavailable
							quantity = (((amount)*conversionfactor))
							if cashonhand >= quantity:
								if amount <= 0:
									message = "I'm sorry, We do not accept checks with a zero balance."
								elif amount == 1:
									message = "Okay, here is your %s %s." %(cointype, cointypecoin)
									inv.Message="CANCELED\n"+inv.Message
									inv.Name = inv.Name + " (CANCELED)"
									bank.withdraw(payer,quantity)
							
									id = activator.Map.CreateObject(cointype1, x, y)
									activator.Take(id)
						
								else:
									message = "Okay, here are your %s %s." %(str(amount), cointypecoin)
									inv.Message="CANCELED\n"+inv.Message
									inv.Name = inv.Name + " (CANCELED)"
									bank.withdraw(payer,quantity)
									id = activator.Map.CreateObject(cointype1, x, y)
									CFItemBroker.Item(id).add(int(amount))
									activator.Take(id)
							
							else:
								message = "I'm sorry, but it appears that %s does not have enough money in his account to cover this transaction." %(payer)
						else:
							message = "The signature on this check is invalid."
							mail.send(1,payer,"The-Imperial-Bank-Of-Skud", "Dear Sir or Madam:\nIt has come to our attention that an attempt to cash an improperly signed check was made.  The check was made out to " + payee + "and the attempt was made by " + activator.Name+".  The value of the check was " + str(amount[1]) +" "+str(amount[2])+' '+str(amount[3])+'.  We apologise for any trouble this may cause and would like to inform you that we are always at your service.'+"\n"+'Sincerly,'+"\n"+'The Imperial Bank of Skud')
			else:
				message="This check has already been cashed!"
		else:
			message = "Come back when you have a check to cash."




	else:
		message = 'Do you need help?'

	whoami.Say(message)
	Crossfire.SetReturnValue(1)
