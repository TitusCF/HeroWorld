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

activator = Crossfire.WhoIsActivator()
activatorname = activator.Name
whoami = Crossfire.WhoAmI()
x = activator.X
y = activator.Y

itotal = 0
ibankdatabase = "ImperialBank_DB"
ibank = CFBank.CFBank(ibankdatabase)

ntotal = 0
nbankdatabase = "navarianroyalBank_DB"
nbank = CFBank.CFBank(nbankdatabase)

for user in ibank.bankdb:
    itotal += ibank.bankdb[user]

for user in nbank.bankdb:
    ntotal += nbank.bankdb[user]

ivalue = (((itotal*20) / ((itotal * 2) + ntotal ) + (itotal / 100000)) * 222)

#EASILY SETTABLE PARAMETERS
service_charge = 0    # service charges for transactions as a percent
exchange_rate = ivalue # exchange rate of imperial to silver (value 1)
bankdatabase = "ImperialStock_DB"

fees = (service_charge/100.0)+1
bank = CFBank.CFBank(bankdatabase)

text = string.split(Crossfire.WhatIsMessage())
thanks_message = [ \
	'Thank you for investing the Imperial Way.', \
	'Thank you for investing the Imperial Way.', \
	'Thank you, please come again.', \
	'Thank you, please come again.', \
	'Thank you for your patronage.', \
	'Thank you for your patronage.', \
	'Thank you, have a nice day.', \
	'Thank you, have a nice day.', \
	'Thank you. "Service" is our middle name.', \
	'Thank you. "Service" is our middle name.', \
	'Thank you. Hows about a big slobbery kiss?' ]


if text[0] == 'help' or text[0] == 'yes':
	message ='You can:\n-deposit,-withdraw,-balance,-exchange \
		\nAll transactions are in Imperial Stock Certificates\n(1 : %d gold coins). \
		\nA service charge of %d percent will be placed on all deposits.' \
		%((exchange_rate/10), service_charge)


elif text[0] == 'deposit':
	if len(text) == 2:
		amount = int(text[1])
		if amount <= 0:
			message = 'Usage "deposit <amount in Imperial Stock Certificates>"'
		elif amount > 10000:
			message = 'Sorry, we do not accept more than 10000 Imperial Stock Certificates for one deposit.'
		elif activator.PayAmount(int(amount*exchange_rate*fees)):
			bank.deposit(activatorname, amount)
			message = '%d platinum received, %d Imperial Stock Certificates deposited to broker account. %s' \
			%((amount*(exchange_rate/50))*fees, amount, random.choice(thanks_message))
		else:
			message = 'You would need %d gold'%((amount*(exchange_rate/10))*fees)
	else:
		message = 'Usage "deposit <amount in Imperial Stock Certificates>"'


elif text[0] == 'withdraw':
	if len(text) == 2:
		amount = int(text[1])
		if amount <= 0:
			message = 'Usage "withdraw <amount in Imperial Stock Certificates>"'
		elif amount > 10000:
			message = 'Sorry, we do not accept more than 10000 Imperial Stock Certificates for one withdraw.'
		elif bank.withdraw(activatorname, amount):
			message = '%d Imperial Stock Certificates withdrawn from broker account. %s' \
			%(amount, random.choice(thanks_message))
			id = activator.Map.CreateObject('istock', x, y)
			CFItemBroker.Item(id).add(amount)
			activator.Take(id)
		else:
			message = 'Not enough Imperial Stock Certificates on your account'
	else:
		message = 'Usage "withdraw <amount in Imperial Stock Certificates>"'


elif text[0] == 'exchange':
	if len(text) == 2:
		amount = int(text[1])
		if amount <= 0:
			message = 'Usage "exchange <amount>" (Imperial Stock Certificates to platinum coins)'
		elif amount > 10000:
			message = 'Sorry, we do not exchange more than 10000 Imperial Stock Certificates all at once.'
		else:
			inv = activator.CheckInventory('istock')
			if inv:
				pay = CFItemBroker.Item(inv).subtract(amount)
				if pay:

					# Drop the coins on the floor, then try
					# to pick them up. This effectively
					# prevents the player from carrying too
					# many coins.
					id = activator.Map.CreateObject('platinum coin', x, y)
					CFItemBroker.Item(id).add(amount*(exchange_rate/50))
					activator.Take(id)

					message = random.choice(thanks_message)
				else:
					message = 'Sorry, you do not have %d Imperial Stock Certificates'%(amount)
			else:
				message = 'Sorry, you do not have any Imperial Stock Certificates'
	else:
		message = 'Usage "exchange <amount>" (Imperial Stock Certificates to platinum coins)'


elif text[0] == 'balance':
	balance = bank.getbalance(activatorname)
	if balance == 1:
		message = 'Amount in broker account: 1 Imperial Stock Certificate'
	elif balance:
		message = 'Amount in broker account: %d Imperial Stock Certificates'%(balance)
	else:
		message = 'Sorry, you have no balance.'


else:
	message = 'Do you need investment help?'

whoami.Say(message)
Crossfire.SetReturnValue(1)
