'''
This script is part of the Witherspoon quest, that starts in /scorn/mansion/witherspoon_manor_attic.
Check the README file in the same directory as this file for more details.

Script for the shop owner of the weapon shop in Scorn.

Will handle the 'give' event to examine the dagger, and various events.

This script should be called for the 'TIMER', 'TIME' and 'custom' events.
'''

import Crossfire

whoami = Crossfire.WhoAmI()
whoisother = Crossfire.WhoIsOther()
command = Crossfire.WhatIsMessage()
event = Crossfire.WhatIsEvent()
pl = Crossfire.WhoIsActivator()

#Message color for player
color = Crossfire.MessageFlag.NDI_BROWN

def do_give():
	'''Player gives an item to the owner.'''
	if whoisother.ReadKey('special_item') != 'ghost_dagger':
		whoami.Say('Nice %s.'%whoisother.Name)
		return

	whoami.Say('Oh, this looks like a really interesting dagger.')
	pl.Message('The owner takes the %s and starts examining it carefully.'%whoisother.Name, color)
	whoami.CreateTimer(8, 1)
	whoami.WriteKey('examining_item', '1', 1)
	whoami.WriteKey('examining_for', pl.Name, 1)

	return

def do_timer():
	'''Owner finished examining the item.'''
	whoami.WriteKey('examining_item', '0', 1)

	#let's see if the player is still around
	pl = Crossfire.FindPlayer(whoami.ReadKey('examining_for'))
	if pl == None:
		# just in case someone else is around ^_-
		whoami.Say('Tss, people just aren\'t patient...')
		return

	whoami.Say('This dagger is pretty old, and is covered with runes. Unfortunately I can\'t describe what they mean, but they sure look like religious ones. Maybe you could go ask a priest?')

if command == 'give':
	if whoami.ReadKey('examining_item') == '1':
		pl.Message('The owner is busy.', color)
	else:
		do_give()
elif event.Subtype == Crossfire.EventType.TIMER:
	do_timer()
elif event.Subtype == Crossfire.EventType.TIME:
	if whoami.ReadKey('examining_item') == '1':
		#No moving while examining.
		Crossfire.SetReturnValue(1)
