# Script for Farnass the cook (/scorn/misc/scorn_kitchen).
#
# The script assumes you have:
# * a cook
# * a recipient where the player will put the ingredients
# * a stove where Farnass will go to cook
#
# Copyright 2007 Nicolas Weeger
# Released as GPL
#
# This script is supposed to be called for the say and time events of the cook,
# and the close event of the recipient.

import Crossfire
import random
import CFMove
from CFDialog import DialogRule, Dialog

quest_name = "wolfsburg/Lursendis"

key_status = 'cook_status'
st_getting = 'getting'		# moving to recipient to get the ingredients
st_stove = 'stove'		# moving to the stove to cook
st_cooking = 'cooking'		# actually cooking

key_cooking_step = 'cooking_step'	# when st_cooking, what cooking step

key_need_check = 'check_ingredients'	# used to signal, after recipient was closed, to check contents

color = Crossfire.MessageFlag.NDI_GREEN	# color to display messages

recipe_arch = 'caramel'			# archetype generated when recipe is complete
eggs_count = 4
failure_chance = 15
recipient_x = 3				# position of the recipient to drop ingredients into
recipient_y = 8
stove_x = 4				# position of the stove the cook will use
stove_y = 8

def check_ingredients():
	''' Finds the ingredients in the recipient. Used when recipient is closed, and when Farnass
	 arrives on the recipient (to avoid player picking up items)'''
	mushroom = None
	eggs = None

#	whoami.Say('before')
	obj = whoami.Map.ObjectAt(recipient_x, recipient_y)
	while obj != None:
		if obj.Type == Crossfire.Type.CONTAINER:
#			whoami.Say('got container %s'%obj.Name)
			inv = obj.Inventory
			while inv != None:
				if inv.Name == 'blue mushroom' and inv.ArchName == 'mushroom_3':
					mushroom = inv
				elif inv.ArchName == 'chicken_egg' and inv.NamePl == 'Combat Chicken eggs':
					eggs = inv
				if mushroom != None and eggs != None:
					break
				inv = inv.Below
			break
		obj = obj.Above
	#whoami.Say('after')

	#if mushroom != None:
	#	whoami.Say('got mushroom')
	#if eggs != None:
	#	whoami.Say('got eggs')

	if mushroom == None or eggs == None or eggs.Quantity < eggs_count:
		if whoami.ReadKey(key_status) == st_getting:
			whoami.Say('Haha, you tried to trick me!')
			whoami.WriteKey(key_status, '', 1)
		return

	if whoami.ReadKey(key_status) != st_getting:
		whoami.Say('Oh, great, you found what I need to make my special caramel!')
		whoami.WriteKey(key_status, st_getting, 1);
		return

	# if called here, Farnass moved to the recipient, and can pick the ingredients
	whoami.Map.Print('%s expertly opens the frypan with a leg, and grabs the ingredient using two sticks in his mouth!'%whoami.Name, color)

	mushroom.Quantity = mushroom.Quantity - 1
	eggs.Quantity = eggs.Quantity - eggs_count

	Crossfire.SetReturnValue(1)
	whoami.WriteKey(key_status, st_stove, 1)

def end_cooking(success):
	''' Everything is finish, let's decide if cooking was successful or not.'''
	whoami.WriteKey(key_status, '', 1)
	whoami.WriteKey(key_cooking_step, '', 1)

	if success == 0:
		return
	if random.randint(1, 100) < failure_chance:
		whoami.Map.Print('%s throws the ingredients in the bin.'%whoami.Name)
		whoami.Say('I can *tell* you shook the eggs. The yellows were so badly stressed inside that this could not work, even in my own hands... err, teeth.')
	else:
		whoami.Say('The caramel is ready!')
		omelet = whoami.Map.CreateObject(recipe_arch, whoami.X, whoami.Y)
		omelet.Name = 'Farnass\'s Special Caramel'
		omelet.NamePl = 'Farnass\'s Special Caramels'
		omelet.Slaying = 'Farnass\'s Special Caramel'
		omelet.Quantity = 1

        # quest advancer
        event = omelet.CreateObject("event_pickup")
        event.Name = "wolfsburg/Lursendis 40>70"
        event.Title = "Python"
        event.Slaying = "/python/quests/QuestAdvance.py"

def close_boiler():
	'''Just tell the cook to check next time.'''
	Crossfire.GetPrivateDictionary()[key_need_check] = 'yes'

def clean_check():
	'''Cancel next check.'''
	d = Crossfire.GetPrivateDictionary()
	if key_need_check in d:
		del d[key_need_check]

def move_cook():
	'''Main moving routine.'''

	#whoami.Say('move')
	status = whoami.ReadKey(key_status)
	if status == st_getting:
		clean_check()
		Crossfire.SetReturnValue(1)
		m = CFMove.get_object_to(whoami, recipient_x, recipient_y)
		if m == 0:
			check_ingredients()
		elif m == 2:
			whoami.Say('Get off my way! You want me to cook this caramel or what?')
		return

	if status == st_cooking:
		clean_check()
		Crossfire.SetReturnValue(1)

		if whoami.X != stove_x or whoami.Y != stove_y:
			whoami.Say('You fool! The ingredients are wasted, now!')
			end_cooking(0)
			return

		step = int(whoami.ReadKey(key_cooking_step)) - 1
		if step == 0:
			end_cooking(1)
			return
		elif step == 15:
			whoami.Map.Print('%s skillfully mixes the ingredients with his left toe while controlling the fire under the boiler with his right one!'%whoami.Name, color)
		elif step == 40:
			whoami.Map.Print('Knife griped by the mouth, %s cuts the mushroom in small slices, and puts them in the stove!'%whoami.Name, color)
			whoami.WriteKey(key_cooking_step, str(random.randint(25, 35)), 1)
		elif step == 50:
			whoami.Say('Let\'s pour the sulphur in now! Hey, wait, did I say sulphur? Ok, getting some dwarven shampoo to neutralize this!')
		elif step == 70:
			whoami.Map.Print('%s breaks the eggs over the stove by expertly throwing them, the shells bouncing away!'%whoami.Name, color)
			whoami.WriteKey(key_cooking_step, str(random.randint(60, 69)), 1)
		whoami.WriteKey(key_cooking_step, str(step), 1)
		return

	if status == st_stove:
		clean_check()
		move = CFMove.get_object_to(whoami, stove_x, stove_y)
		if move == 0:
			whoami.WriteKey(key_cooking_step, str(random.randint(80, 100)), 1)
			whoami.Map.Print('%s makes the stove hotter.'%whoami.Name, color)
			whoami.WriteKey(key_status, st_cooking, 1)
		elif move == 2:
			whoami.Say('Get off my way, I need to get to the stove!')
		Crossfire.SetReturnValue(1)
		return

	d = Crossfire.GetPrivateDictionary()
	if key_need_check in d:
		whoami.Map.Print('You see %s look at the frypan.'%whoami.Name, color)
		del d[key_need_check]
		check_ingredients()
		return

def cook_talk():
    speech = Dialog(Crossfire.WhoIsActivator(), Crossfire.WhoAmI(), "scorn/Farnass")

    idx = 1

    match = ["eggs"]
    pre = [["quest", quest_name, "=40"]]
    msg = ["My friend Sentrio lives somewhere in Lake Country, but I don't remember where exactly, sorry...\n\nI do know eggs from his chicken are the only ones worth my cooking skill!"]
    post = []
    replies = [
        ["mushroom", "Where can I find the mushroom?", 2],
        ["cook", "But how can you cook without arms?", 2]]
    speech.addRule(DialogRule(match, pre, msg, post, replies),idx)
    idx = idx + 1

    match = ["mushroom"]
    pre = [["quest", quest_name, "=40"]]
    msg = ["It's a blue one, I think it grows in a marsh in the west. I've heard it can be used for medicine, too."]
    post = []
    replies = [
        ["eggs", "Where can I find the eggs?", 2],
        ["cook", "But how can you cook without arms?", 2]]
    speech.addRule(DialogRule(match, pre, msg, post, replies),idx)
    idx = idx + 1

    match = ["cook"]
    pre = [["quest", quest_name, "=40"]]
    msg = ["You wonder how I cook without arms? Heh, rookie, that's what makes the difference between a cooker and me, Farnass! I told you already: I'm simply wonderful."]
    post = []
    replies = [
        ["eggs", "Where can I find the eggs?", 2],
        ["mushroom", "Where can I find the mushroom?", 2]]
    speech.addRule(DialogRule(match, pre, msg, post, replies),idx)
    idx = idx + 1

    match = ["*"]
    pre = [["quest", quest_name, "=40"]]
    msg = ["If you really want this caramel, then bring me the ingredients, please."]
    post = []
    replies = [
        ["mushroom", "Where can I find the mushroom?", 2],
        ["eggs", "Where can I find the eggs?", 2],
        ["cook", "But how can you cook without arms?", 2]]
    speech.addRule(DialogRule(match, pre, msg, post, replies),idx)
    idx = idx + 1

    match = ["ingredients"]
    pre = [["quest", quest_name, "=10"], ["token", "dialog", "4"]]
    msg = ["You need the following:\n- 4 eggs from my friend Sentrio's chicken\n- a blue mushroom\n\nAs you can see, I've lost my both arms during the last war against the Gnolls, so would you be kind enough to put whatever you found during your travel to make the recipe in the frypan, please?"]
    post = [["settoken", "dialog", "0"], ["quest", quest_name, "40"]]
    replies = [
        ["mushroom", "Where can I find the mushroom?", 2],
        ["eggs", "What kind of eggs?", 2],
        ["cook", "But how can you cook without arms?", 2]]
    speech.addRule(DialogRule(match, pre, msg, post, replies),idx)
    idx = idx + 1

    match = ["caramel"]
    pre = [["quest", quest_name, "=10"], ["token", "dialog", "3"]]
    msg = ["Ha, if my great friend wants a caramel, then I can only oblige!"]
    post = [["settoken", "dialog", "4"]]
    replies = [["ingredients", "So what would the ingredients be?", 2]]
    speech.addRule(DialogRule(match, pre, msg, post, replies),idx)
    idx = idx + 1

    match = ["yes"]
    pre = [["quest", quest_name, "=10"], ["token", "dialog", "2"]]
    msg = ["How obviously from him!\n\nSo, what will he want to eat, this time?"]
    post = [["settoken", "dialog", "3"]]
    replies = [["caramel", "A caramel.", 1]]
    speech.addRule(DialogRule(match, pre, msg, post, replies),idx)
    idx = idx + 1

    match = ["lursendis"]
    pre = [["quest", quest_name, "=10"], ["token", "dialog", "1"]]
    msg = ["Oh, this good friend! I should go see him someday... Let me guess, he wants to eat something, he?"]
    post = [["settoken", "dialog", "2"]]
    replies = [["yes", "Well, yes.", 1]]
    speech.addRule(DialogRule(match, pre, msg, post, replies),idx)
    idx = idx + 1

    match = ["*"]
    pre = [["quest", quest_name, "=10"]]
    msg = ["Please don't disturb me, I'm trying a really hard recipe."]
    post = [["settoken", "dialog", "1"]]
    replies = [["lursendis", "Your friend Lursendis sent me here."]]
    speech.addRule(DialogRule(match, pre, msg, post, replies),idx)
    idx = idx + 1

    match = ["*"]
    pre = []
    msg = ["Please don't disturb me, I'm trying a really hard recipe."]
    post = []
    replies = []
    speech.addRule(DialogRule(match, pre, msg, post, replies),idx)
    idx = idx + 1

    speech.speak(Crossfire.WhatIsMessage())
    Crossfire.SetReturnValue(1)

whoami = Crossfire.WhoAmI()
if Crossfire.WhatIsEvent().Subtype == Crossfire.EventType.SAY:
	if whoami.ReadKey(key_cooking_step) != '':
		Crossfire.NPCSay(whoami, 'Keep quiet, this recipe requires concentration!')
		Crossfire.SetReturnValue(1)
	else:
		cook_talk()
elif Crossfire.WhatIsEvent().Subtype == Crossfire.EventType.TIME:
	move_cook()
elif Crossfire.WhatIsEvent().Subtype == Crossfire.EventType.CLOSE:
	close_boiler()
