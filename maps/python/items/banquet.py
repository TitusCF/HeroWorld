# Script for the Unforgettable Banquet of Lursendis item.
# Idea courtesy Yann Chachkoff.
#
# Copyright 2007 Nicolas Weeger
# Released as GPL
#
# This script will teleport the player to a food-filled map. It only works
# once a (ingame) day.
# Applying the tome while in the map will teleport the player back to his starting place.
# Book can only be applied when in player's inventory, so player can get back to the world.
#
# Map is generated by the script itself, and given a unique name to avoid collision.
#
# Player is replaced with a statue so the spot stays free during his trip.
# To avoid colliding with an existing map, created map will have a unique map.
# Everything in the map is randomly chosen.

import Crossfire
import random
import os.path

# map sizes
size_x = 5
size_y = 5

# what foods to put in the map
foods = [ 'clover2' ]
# what floors can be used
floors = [ 'woodfloor_camp', 'woodfloor_camp1',  'woodfloor_camp2' ]
# what walls can be used - only put the base name
walls = [ 'flagstone', 'dwall', 'timberwall', 'stwall' ]

# what to replace the player with. Should block all movement
replace_with = [ 'statue2_gold' ]

def get_one(what):
	'''Return a random choice in what, which should be an array.'''
	return what[ random.randint( 1, len(what) ) - 1 ]

def return_to_bed():
	'''Teleport player back to bed of reality. Only in emergency.'''
	flags = 0
	if (act.BedMap.find(Crossfire.PlayerDirectory()) != -1):
		# Unique map
		flags = 2
	dest = Crossfire.ReadyMap(act.BedMap, flags)
	if (dest == None):
		# Ok, this is real bad. Let player handle the situation - worse case, call to DMs or WoR manually.
		act.Message('The %s whines really loudly.'%l.Name)
		return

	act.Teleport(dest, act.BedX, act.BedY)

def do_back():
	'''Teleport the player back to his starting point.'''
	x = l.ReadKey('banquet_x')
	y = l.ReadKey('banquet_y')
	mn = l.ReadKey('banquet_map')
	rw = l.ReadKey('banquet_rw')

	l.WriteKey('banquet_x', '', 0)
	l.WriteKey('banquet_y', '', 0)
	l.WriteKey('banquet_map', '', 0)
	l.WriteKey('banquet_rw', '', 0)

	if x == '' or y == '' or mn == '':
		# Logic error, but can't be helped - teleport player back to his bed of reality
		act.Message('You feel a distorsion of reality!')
		return_to_bed()
		return

	# find and remove statue in the map
	dest = Crossfire.ReadyMap(mn)
	if (dest == None):
		# Random map, probably? Let the player teleport back him/herself.
		act.Message('The %s whines something. You barely understand it can\'t take you back to your starting point.'%l.Name)
		return_to_bed()
		return

	# Remove statue - let's assume it's found, or was removed due to map reset.
	st = dest.ObjectAt(int(x), int(y))
	while st != None:
		if (st.ArchName != rw):
			st = st.Above
			continue;
		st.Remove()
		break;

	act.Message('You feel a powerful force engulf you.')
	act.Teleport(dest, int(x), int(y))

def do_banquet():
	'''Teleports the player to the pocket dimension, if not used for one day. '''

	now = str(Crossfire.GetTime()[0]) + '-' + str(Crossfire.GetTime()[1]) + '-' + str(Crossfire.GetTime()[2])

	l = Crossfire.WhoAmI()
	act = Crossfire.WhoIsActivator()

	last = l.ReadKey('banquet_last')


	if (last == now):
		act.Message('You read the %s but nothing happens.'%l.Name)
		return;

	l.WriteKey('banquet_last', now, 1)

	# map generation
	m = Crossfire.CreateMap(size_x, size_y)
	m.Path = os.path.join(Crossfire.ScriptName(), Crossfire.WhoIsActivator().Name)
	floor = get_one(floors)

	# First, let's put the floors
	for x in range(size_x):
		for y in range(size_y):
			fl = Crossfire.CreateObjectByName(floor)
			fl.Teleport(m, x, y)

	# Put walls.
	wall = get_one(walls)
	# top left
	w = Crossfire.CreateObjectByName(wall + '_2_2_2')
	w.Teleport(m, 0, 0)
	# top right
	w = Crossfire.CreateObjectByName(wall + '_2_2_3')
	w.Teleport(m, size_x - 1, 0)
	# bottom left
	w = Crossfire.CreateObjectByName(wall + '_2_2_1')
	w.Teleport(m, 0, size_y - 1)
	# bottom right
	w = Crossfire.CreateObjectByName(wall + '_2_2_4')
	w.Teleport(m, size_x - 1, size_y - 1)
	# top and bottom parts
	for x in range(size_x - 2):
		w = Crossfire.CreateObjectByName(wall + '_2_1_2')
		w.Teleport(m, x + 1, 0)
		w = Crossfire.CreateObjectByName(wall + '_2_1_2')
		w.Teleport(m, x + 1, size_y - 1)
	# left and right parts
	for y in range(size_y - 2):
		w = Crossfire.CreateObjectByName(wall + '_2_1_1')
		w.Teleport(m, 0, y + 1)
		w = Crossfire.CreateObjectByName(wall + '_2_1_1')
		w.Teleport(m, size_x - 1, y + 1)

	# Food itself
	for x in range(size_x-2):
		for y in range(size_y-2):
			fo = Crossfire.CreateObjectByName(get_one(foods))
			fo.GodGiven = 1
			fo.Teleport(m, x + 1, y + 1)

	# Store player's current location
	x = act.X
	y = act.Y
	im = act.Map
	rw = get_one(replace_with)

	l.WriteKey('banquet_x', str(x), 1)
	l.WriteKey('banquet_y', str(y), 1)
	l.WriteKey('banquet_map', im.Path, 1)
	l.WriteKey('banquet_rw', rw, 1)

	# Teleport
	act.Message('You feel grabbed by some powerful force.')
	act.Teleport(m, int(( size_x - 1 ) / 2), int((size_y - 1) / 2))

	# Keep free spot by putting a statue
	statue = im.CreateObject(rw, x, y)
	statue.Name = '%s\'s statue'%act.Name
	statue.Message = 'Is elsewhere...'

l = Crossfire.WhoAmI()
act = Crossfire.WhoIsActivator()

if l.Env != act:
	act.Message('You try to open the %s, but it seems to flee from you.'%l.Name)
elif (act.Map.Path.find(Crossfire.ScriptName()) != -1):
	do_back()
else:
	do_banquet()

Crossfire.SetReturnValue(1)
