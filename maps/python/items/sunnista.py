# Script for the Sunnista bagpipe item.
# Idea courtesy Yann Chachkoff.
#
# Copyright 2007 Nicolas Weeger
# Released as GPL
#
# When this item is applied, mice in the 5x5 square around player are absorbed up to a certain
# number. If no mouse, will generate a friendly mouse whose power is based on the number of absorbed mice
# to this point.
# Only up to a certain number of mice can be absorbed, till the object spits an enemy mouse.

import Crossfire

# what archetype this will affect - only works on items with good archetype and non altered name.
affect = 'mouse'
# plural name, to display absorption
affect_pl = 'mice'
# maximum number of absorbed monsters per use
max_affect = 3
# maximum number of charges
max_charges = 15

def do_release(friendly):
	act = Crossfire.WhoIsActivator()
	l = Crossfire.WhoAmI()

	charges = l.ReadKey('sunnista_charges')
	if (charges != ''):
		charges = int(charges)
	else:
		charges = 0
	if (charges == 0):
		act.Message('The %s vibrates slightly.'%l.Name)
		return

	m = Crossfire.CreateObjectByName(affect)
	m.MaxHP = m.MaxHP * charges
	m.HP = m.MaxHP
	m.Dam = m.Dam * charges
	m.WC = m.WC * charges
	m.AC = m.AC * charges
	if friendly == 1:
		m.IsPet = 1
		m.Owner = act
		m.Friendly = 1
		m.AttackMovement = 16 # petmode
	m.Teleport(act.Map, act.X, act.Y)
	l.WriteKey('sunnista_charges', '0', 1);
	act.Message('The %s spits a %s!'%(l.Name, affect))

def do_absorb(count):
	l = Crossfire.WhoAmI()
	charges = l.ReadKey('sunnista_charges')
	if (charges != ''):
		charges = int(charges)
	else:
		charges = 0

	Crossfire.WhoIsActivator().Message('The %s absorbs some %s!'%(l.Name, affect_pl))

	charges += count
	l.WriteKey('sunnista_charges', str(charges), 1)
	if charges > max_charges:
		do_release(0)

def do_find():
	#global max_affect
	got = 0
	l = Crossfire.WhoAmI()
	act = Crossfire.WhoIsActivator()

	for rx in range(5):
		for ry in range(5):
			item = Crossfire.WhoIsActivator().Map.ObjectAt(act.X + rx - 2, act.Y + ry - 2)
			while item != None:
				if item.ArchName == affect and item.Friendly == 0:
					got = got + 1
					item.Remove()
					break;
				if got > max_affect:
					return got
				item = item.Above
	return got

Crossfire.SetReturnValue(1)

got = do_find()
if got == 0:
	do_release(1)
else:
	do_absorb(got)
