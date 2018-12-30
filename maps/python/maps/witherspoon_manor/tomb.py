'''
This script is part of the Witherspoon quest, that starts in /scorn/mansion/witherspoon_manor_attic.
Check the README file in the same directory as this file for more details.

Script for the tomb near the lake west of Scorn.

This script is called when the player steps on the correct spot where the body is buried.
'''

import Crossfire

def can_dig(pl):
	'''Returns True if the player can dig, False else. Give the relevant message.'''
	if pl.CheckArchInventory('skill_clawing') != None:
		pl.Write('Using your claws, you quickly dig.')
		return True
	if pl.CheckArchInventory('shovel_1') != None:
		pl.Write('You dig with your shovel.')
		return True

	pl.Write('You\'d dig, but you have nothing to dig with...')
	return False

def find_player():
	'''Find the player stepping on the detector'''
	test = Crossfire.WhoAmI().Above
	while test != None:
		if test.Type == Crossfire.Type.PLAYER:
			return test
		test = test.Above
	return None

def main():
	pl = find_player()
	if pl == None:
		return

	if pl.ReadKey('dialog_witherspoon_ghost') != 'witherspoon_ghost:wait':
		return

	if pl.ReadKey('witherspoon_tomb') != '':
		# Already dig, no need to give more items
		return

	pl.Write('You notice the earth here is kind of bumpy.')

	#ok, so two choices for the player: if she got clawing, easy to dig. Else need a shovel.
	dig = can_dig(pl)
	if dig == 0:
		return

	#don't want the player to dig again! Will be reset by the ghost later on
	pl.WriteKey('witherspoon_tomb', 'dig', 1)

	body = Crossfire.CreateObjectByName('corpse') # so it doesn't merge with another item
	body.WriteKey('special_item', 'ghost_body', 1)
	body.Name = 'tortured body'
	body.NamePl = 'tortured bodies'
	body.Message = 'You suppose this is the body of the ghost in Witherspoon Manor. It is covered in scars, as if someone really wanted to make him pay for something.'
	body.InsertInto(pl)

	dagger = Crossfire.CreateObjectByName('dagger')
	dagger.WriteKey('special_item', 'ghost_dagger', 1)
	dagger.Name = 'strange dagger'
	dagger.NamePl = 'strange daggers'
	dagger.Message = 'You found this dagger with the body of the Witherspoon Manor ghost. It has some weird look. You wonder if a marchant could figure what the symbols mean.'
	dagger.InsertInto(pl)

	pl.Write('You find a body with a dagger in it!')

main()
