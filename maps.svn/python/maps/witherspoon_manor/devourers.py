'''
This script is part of the Witherspoon quest, that starts in /scorn/mansion/witherspoon_manor_attic.
Check the README file in the same directory as this file for more details.

Script for the priest of Devourers in Scorn.

Will handle the 'give' event to examine the dagger or money, and various events.

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
	'''Player gives an item to the priest.'''
	if whoami.ReadKey('got_offrand') == '':
		if whoisother.ArchName == 'platinacoin':
			if whoisother.Quantity < 10:
				pl.Message('The priest looks at your money and sighes deeply, but doesn\'t take it.', color)
				return

			rem = whoisother.Quantity
			if whoisother.Quantity > 50:
				rem = 50
				whoami.Say('Now, now, I can\' accept all that money.')
				pl.Message('The priest takes 50 platinum coins.', color)
			else:
				pl.Message('The priest accepts your money.', color)

			whoisother.Quantity = whoisother.Quantity - rem

			whoami.WriteKey('got_offrand', '1', 1)
			whoami.Say('Many thanks for your offrand!')

			return

		whoami.Say('Our church needs some restoration, could you donate to it?')

		return

	whoami.WriteKey('got_offrand', '', 1)

	if whoisother.ReadKey('special_item') != 'ghost_dagger':
		whoami.Say('Nice %s.'%whoisother.Name)
		return

	whoami.Say('My, my, those runes are pretty interesting.')
	pl.Message('The priest takes the %s and looks at it carefully.'%whoisother.Name, color)
	whoami.CreateTimer(8, 1)
	whoami.WriteKey('examining_item', '1', 1)
	whoami.WriteKey('examining_for', pl.Name, 1)

	return

def do_timer():
	'''Priest finished examining the item.'''
	whoami.WriteKey('examining_item', '0', 1)

	#let's see if the player is still around
	pl = Crossfire.FindPlayer(whoami.ReadKey('examining_for'))
	if pl == None:
		# just in case someone else is around ^_-
		whoami.Say('Tss, people just aren\'t patient...')
		return

	pl.WriteKey('witherspoon_know_dagger', '1', 1)
	whoami.Say('As far as I can tell, the runes on this dagger represent an invocation to our Lord. It is meant to keep the soul in the body for hours after the death.')
	whoami.Map.Print('The priest shudders.', color)
	whoami.Say('The victim of the spell can still feel pain, even agonizing one, but will not die before the spell effect wears out.')
	whoami.Say('Even our Lord, in his mercy, does not inflict on his followers such agonizing pain.')
	whoami.Say('For someone to use such a spell, the victim must have been so hated!')
	whoami.Map.Print('The priest thinks for a few seconds.', color)
	whoami.Say('I think there is a witch, Olandi or something, who lives south of Scorn and specializes in Devourers magic.')
	whoami.Say('Maybe she could help you?')

if command == 'give':
	if whoami.ReadKey('examining_item') == '1':
		pl.Message('The priest is busy.', color)
	else:
		do_give()
elif event.Subtype == Crossfire.EventType.TIMER:
	do_timer()
elif event.Subtype == Crossfire.EventType.TIME:
	if whoami.ReadKey('examining_item') == '1':
		#No moving while examining.
		Crossfire.SetReturnValue(1)
