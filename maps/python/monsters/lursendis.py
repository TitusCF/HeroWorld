# Script for Lursendis the gourmet (/wolfsburg/lursendis).
# Idea courtesy Yann Chachkoff.
#
# The script assumes you have:
# * Lursendis
# * a place where the player will drop the special omelet
#
# Copyright 2007 Nicolas Weeger
# Released as GPL
#
# This script is supposed to be called for the time event of Lursendis.

import Crossfire
import CFMove
import random
from CFDialog import DialogRule, Dialog

quest_name = "wolfsburg/Lursendis"

key_status = 'gourmet_status'
st_getting = 'getting'
st_eating = 'eating'
key_eating_step = 'eating_step'
plate_x = 2
plate_y = 6

banquet_path = '/python/items/banquet.py'
banquet_archetype = 'tome'
event_archetype = 'event_apply'

color = Crossfire.MessageFlag.NDI_GREEN	# color to display messages

def check_plate():
	obj = whoami.Map.ObjectAt(plate_x, plate_y)
	while obj != None:
		if obj.NamePl == 'Farnass\'s Special Caramels' and obj.Slaying == 'Farnass\'s Special Caramel':
			if whoami.ReadKey(key_status) == st_getting:
				whoami.Map.Print('%s grabs a %s and starts eating with an obvious pleasure.'%(whoami.Name, obj.Name))
				obj.Quantity = obj.Quantity - 1
				whoami.WriteKey(key_status, st_eating, 1)
				whoami.WriteKey(key_eating_step, str(random.randint(5, 10)), 1)
				Crossfire.SetReturnValue(1)
				return

			whoami.Say('Oh! Could this be...')
			whoami.WriteKey(key_status, st_getting, 1)
			Crossfire.SetReturnValue(1)
			return

		obj = obj.Above

	if whoami.ReadKey(key_status) == st_getting:
		# we were on the spot, but no more omelet...
		whoami.WriteKey(key_status, '', 1)

def create_book():
	book = whoami.Map.CreateObject(banquet_archetype, whoami.X, whoami.Y)
	book.Name = 'Unforgettable Banquet of %s'%whoami.Name
	book.NamePl = 'Unforgettable Banquets of %s'%whoami.Name
	event = book.CreateObject(event_archetype)
	event.Slaying = banquet_path
	event.Title = Crossfire.WhatIsEvent().Title

def move_gourmet():
	st = whoami.ReadKey(key_status)
	if st == st_getting:
		move = CFMove.get_object_to(whoami, plate_x, plate_y)
		if move == 0:
			check_plate()
			return
		elif move == 2:
			whoami.Say('Get outta my way!')
		Crossfire.SetReturnValue(1)
		return
	elif st == st_eating:
		step = int(whoami.ReadKey(key_eating_step)) - 1
		if step == 0:
			whoami.WriteKey(key_eating_step, '', 1)
			whoami.WriteKey(key_status, '', 1)
			whoami.Say('Now that\'s what I call a caramel! Thank you very much!')
			whoami.Say('Here, take this as a token of my gratitude.')
			create_book()
			for pl in Crossfire.GetPlayers():
			    if pl.Map == whoami.Map and pl.QuestGetState(quest_name) == 70:
			        pl.QuestSetState(quest_name, 100)
			return
		whoami.WriteKey(key_eating_step, str(step), 1)
		Crossfire.SetReturnValue(1)
		return

	check_plate()

def talk_gourmet():
    pl = Crossfire.WhoIsActivator()
    speech = Dialog(Crossfire.WhoIsActivator(), Crossfire.WhoAmI(), quest_name)
    completed = pl.QuestWasCompleted(quest_name)

    idx = 1

    prer = [["quest",quest_name, "10"]]
    rmsg = ["So, do you have a caramel made by Farnass? If so, please put it on the plate, I'm so hungry!"]
    postr = []
    speech.addRule(DialogRule(["*"], prer, rmsg, postr),idx)
    idx = idx + 1

    prer = [["quest",quest_name, "0"], ["token", "asked", "1"]]
    postr = [["settoken", "asked", "0"]]
    rmsg = ["Ha well, too bad... If you ever change your mind, please tell me!"]
    speech.addRule(DialogRule(["no"], prer, rmsg, postr),idx)
    idx = idx + 1

    if completed:
        next = "40"
    else:
        next = "10"

    prer = [["quest",quest_name, "0"], ["token", "asked", "1"]]
    postr = [["settoken", "asked", "0"], ["quest", quest_name, next]]
    rmsg = ["Thank you very much!"]
    speech.addRule(DialogRule(["yes"], prer, rmsg, postr),idx)
    idx = idx + 1

    if completed:
        prer = [["quest",quest_name, "0"]]
        postr = [["settoken", "asked", "1"]]
        rmsg = ["Hum, I'm still hungry, I could use another caramel from Farnass... Could you get me another one, please?"]
        replies = [["yes", "Sure"], ["no", "Sorry, I'm really busy now, I don't have time..."]]
        speech.addRule(DialogRule(["*"], prer, rmsg, postr, replies),idx)
        idx = idx + 1
    else:
        prer = [["quest", quest_name, "0"], ["token", "dialog", "3"]]
        postr = [["settoken", "asked", "1"], ["settoken", "dialog", "0"]]
        rmsg = ["Would you really be as kind as that?"]
        replies = [["yes", "If you really need one caramel, yes, sure."], ["no", "Well, no, I was just joking."]]
        speech.addRule(DialogRule(["bring"], prer, rmsg, postr, replies),idx)
        idx = idx + 1

        prer = [["quest", quest_name, "0"], ["token", "dialog", "2"]]
        postr = [["settoken", "dialog", "3"]]
        rmsg = ["Farnass 'The Recipe Spellcrafter'. Good friend, haven't seen him in 15 years...\nI think he lived in Scorn, or some island around."]
        replies = [["bring", "Should I get you one of his caramels, then?", 2]]
        speech.addRule(DialogRule(["farnass"], prer, rmsg, postr, replies),idx)
        idx = idx + 1

        prer = [["quest", quest_name, "0"], ["token", "dialog", "1"]]
        postr = [["settoken", "dialog", "2"]]
        rmsg = ["Yes, but I would only eat a caramel made by my friend Farnass."]
        replies = [["farnass", "Who is Farnass?", 2]]
        speech.addRule(DialogRule(["caramel"], prer, rmsg, postr, replies),idx)
        idx = idx + 1

        prer = [["quest", quest_name, "0"]]
        postr = [["settoken", "dialog", "1"]]
        rmsg = ["I'm hungry, I could use a caramel."]
        replies = [["caramel", "A caramel, really?", 2]]
        speech.addRule(DialogRule(["*"], prer, rmsg, postr, replies),idx)
        idx = idx + 1

    speech.speak(Crossfire.WhatIsMessage())
    Crossfire.SetReturnValue(1)

whoami = Crossfire.WhoAmI()
if Crossfire.WhatIsEvent().Subtype == Crossfire.EventType.TIME:
    move_gourmet()
elif Crossfire.WhatIsEvent().Subtype == Crossfire.EventType.SAY:
    talk_gourmet()
