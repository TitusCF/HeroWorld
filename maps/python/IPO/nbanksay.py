# Script for say event of navarianroyal Bank Tellers
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


#EASILY SETTABLE PARAMETERS
service_charge = 1    # service charges for transactions as a percent
exchange_rate = 5283 # exchange rate of navarianroyal to silver (value 1)
bankdatabase = "navarianroyalBank_DB"

fees = (service_charge/100.0)+1
bank = CFBank.CFBank(bankdatabase)

text = string.split(Crossfire.WhatIsMessage())
thanks_message = [ \
	'Thank you for banking with the Bank of Navar', \
	'Thank you for banking with the Bank of Navar', \
	'Thank you, please come again.', \
	'Thank you, please come again.', \
	'Thank you for your patronage.', \
	'Thank you for your patronage.', \
	'Thank you, have a nice day.', \
	'Thank you, have a nice day.', \
	'Thank you. "Service" is our middle name.', \
	'Thank you. "Service" is our middle name.', \
	'Thank you.' ]


if text[0] == 'help' or text[0] == 'yes':
	message ='You can:\n-deposit,-withdraw,-balance,-exchange \
		\nAll transactions are in navarian royals\n(1 : %d gold coins). \
		\nA service charge of %d percent will be placed on all deposits.' \
		%((exchange_rate/10), service_charge)


elif text[0] == 'deposit':
	if len(text) == 2:
		amount = int(text[1])
		if amount <= 0:
			message = 'Usage "deposit <amount in navarian royals>"'
		elif amount > 10000:
			message = 'Sorry, we do not accept more than 10000 navarian royals for one deposit.'
		elif activator.PayAmount(int(amount*exchange_rate*fees)):
			bank.deposit(activatorname, amount)
			message = '%d platinum received, %d navarian royals deposited to bank account. %s' \
			%((amount*(exchange_rate/50))*fees, amount, random.choice(thanks_message))
		else:
			message = 'You would need %d gold'%((amount*(exchange_rate/10))*fees)
	else:
		message = 'Usage "deposit <amount in navarian royals>"'


elif text[0] == 'withdraw':
	if len(text) == 2:
		amount = int(text[1])
		if amount <= 0:
			message = 'Usage "withdraw <amount in navarian royals>"'
		elif amount > 10000:
			message = 'Sorry, we do not accept more than 10000 navarian royals for one withdraw.'
		elif bank.withdraw(activatorname, amount):
			message = '%d navarian royals withdrawn from bank account. %s' \
			%(amount, random.choice(thanks_message))
			id = activator.Map.CreateObject('navarianroyal', x, y)
			CFItemBroker.Item(id).add(amount)
			activator.Take(id)
		else:
			message = 'Not enough navarian royals on your account'
	else:
		message = 'Usage "withdraw <amount in navarian royals>"'


elif text[0] == 'exchange':
	if len(text) == 2:
		amount = int(text[1])
		if amount <= 0:
			message = 'Usage "exchange <amount>" (navarian royals to platinum coins)'
		elif amount > 10000:
			message = 'Sorry, we do not exchange more than 10000 navarian royals all at once.'
		else:
			inv = activator.CheckInventory('navarianroyal')
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
					message = 'Sorry, you do not have %d navarian royals'%(amount)
			else:
				message = 'Sorry, you do not have any navarian royals'
	else:
		message = 'Usage "exchange <amount>" (navarian royals to platinum coins)'


elif text[0] == 'balance':
	balance = bank.getbalance(activatorname)
	if balance == 1:
		message = 'Amount in bank: 1 navarian royal'
	elif balance:
		message = 'Amount in bank: %d navarian royals'%(balance)
	else:
		message = 'Sorry, you have no balance.'


else:
	message = 'Do you need help?'

whoami.Say(message)
Crossfire.SetReturnValue(1)
