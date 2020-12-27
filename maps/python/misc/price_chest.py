# Intended to be attached to a say_event inside a chest, inside a shop.  Lets a player price an item in gold, put that item up for sale, and then receive the amount they requested.
# If we can find a way to get the final price of the item, this could be easily changed to give the seller that amount instead.
# Created by James W Bennett in 2020

import Crossfire
import CFBank

scob = Crossfire.WhoAmI()
act = Crossfire.WhoIsActivator()
eve = Crossfire.WhatIsEvent()

def set_price():
	invcount = 0
	chestinv = scob.Inventory
	while chestinv != None:#counts the number of items in the chest
		invcount += 1
		chestinv = chestinv.Below
	chestinv = scob.Inventory # Resets which item the script is looking at
	if invcount > 2: # Runs if there's more than just the event plus one item in the chest.
		act.Message("Only one item may be priced at a time.")
	elif invcount < 2:
		act.Message("Drop an item inside, then say the desired price in gold pieces.")
	elif ((chestinv.Unpaid is 1) and (str(act.Name) != str(chestinv.ReadKey('player_seller')))): #Triggers if the item is unpaid and the player isn't the person who put the item up for sale
		act.Message("This item is already for sale.")
	elif ((chestinv.Unpaid is 1) and (str(act.Name) == str(chestinv.ReadKey('player_seller')))): #Triggers if the player already put this item up for sale.  We strip the item, returning it to its original state.
		strip_item(chestinv)
		act.Message("You are no longer offering this item for sale.")
		act.Take(chestinv)
	elif chestinv.Name == "event_say": #Just in case the item order gets messed up somehow, causing us to point to the wrong thing
		act.Message("Please pick up and re-drop the item before continuing.")
	else:
		datadir = Crossfire.DataDirectory()
		mapdir = Crossfire.MapDirectory()
		try:#Should only trigger if the user message was an integer
			named_value = int(Crossfire.WhatIsMessage()) * 5 #We turn these into ints to make sure the player named a price.  x5 because 1 CF "value" is 2 silver (for some reason)
			original_value = int(chestinv.Value)#stores the original value of the item, so we can reset it later.
			player_seller = str(act.Name)#stores the player selling the item, so we can pay them later
			chestinv.WriteKey('original_value', str(original_value), 1)
			chestinv.WriteKey('named_value', str(named_value), 1)
			chestinv.WriteKey('player_seller', player_seller, 1)
			chestinv.Value = named_value
			chestinv.Unpaid = 1
			sellev = Crossfire.CreateObjectByName('event_bought')# The script that we attach to the item, to handle the sale
			sellev.Title = 'Python'
			sellev.Slaying = str(Crossfire.ScriptName()).replace((str(datadir) + '/' + str(mapdir)), '')# This should point to the path of this script
			sellev.InsertInto(chestinv)
			act.Take(chestinv)
		except (TypeError, ValueError):#Triggers if the user said something other than an integer.
			act.Message("Please state the desired price of the item, in gold pieces.")
	
def do_buy():
	player_seller = str(scob.ReadKey('player_seller'))
	named_value = int(scob.ReadKey('named_value')) * 2 #Because 1 CF "value" is 2 silver, we need to deposit 2x the value into the player's account.
	with CFBank.CFBank("ImperialBank_DB") as bank:
		bank.deposit(player_seller, named_value)
	act.Message((str(int(named_value) / 10)) + " gold from the sale went to " + player_seller + ".")
	strip_item(scob)
	
def strip_item(strob):
	player_seller = str(strob.ReadKey('player_seller'))
	named_value = int(strob.ReadKey('named_value'))
	strob.Value = int(strob.ReadKey('original_value'))
	strob.WriteKey('player_seller', '', 1)
	strob.WriteKey('named_value', '', 1)
	strob.WriteKey('original_value', '', 1)
	if eve.Subtype is 34: #We only want to remove the event if it's the kind we attach to items.
		eve.Remove()
	else: #This should fire when a player is trying to reprice an item.
		strobinv = strob.Inventory
		while ((strobinv != None) and (strobinv.Name != "event_bought")):#iterates through the strip-item's inventory, looking for either the end of the list or the event we're trying to strip
			strobinv = strobinv.Below
		while strobinv.Name == "event_bought":#we remove all side-by-side instances of event_bought, just in case an item somehow got multiple.
			strobinv.Remove()
		strob.Unpaid = 0


if eve.Subtype is 6:#Checks if the event is event_say (subtype 6 means event_say, which is when the player speaks to the chest)
	set_price()

elif eve.Subtype is 34:#Subtype 34 is the new event_bought, courtesy of SilverNexus
	do_buy()
