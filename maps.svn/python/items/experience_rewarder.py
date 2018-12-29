# Script to give experience to a player.
#
# Copyright 2007 Nicolas Weeger
# Released as GPL
#
# This script will give experience to the "activating" player.
# It should be linked through the "trigger" event of an altar, or similar thing.
# Options are specified in the event object's fields.
# A key/value will be used to store in the player if she activated the item already.
#
# Available options are:
# - exp: experience to gain
# - skill: skill to add experience to. Can be empty, in which case exp is given to the general exp only
# - wc: what to do when the player doesn't know the skill:
#   - 0: give the player the skill
#   - 1: give player exp to total, no skill
#   - 2: player gets nothing
# - race: if set, the player can only use once this item.

import Crossfire

key_prefix = 'experience_rewarder_'

def do_give_exp():
	pl = Crossfire.WhoIsActivator()
	evt = Crossfire.WhatIsEvent()

	if evt.Race != None and evt.Race != '':
		if pl.ReadKey(key_prefix + evt.Race) != '':
			return
		pl.WriteKey(key_prefix + evt.Race, 'used', 1)

	if evt.Skill == None or evt.Skill == '':
		pl.AddExp(evt.Exp)
		return

	wc = evt.WC
	if wc < 0 or wc > 2:
		wc = 1
	pl.AddExp(evt.Exp, evt.Skill, wc)


do_give_exp()
