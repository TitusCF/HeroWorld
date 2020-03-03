# Script for the Ship's Cabin functionality
#
# Written by James W. Bennett in 2020, based on Nicolas Weeger's "banquet.py" script from 2007
# Released as GPL
# Original concept/reason for writing was Titus', for their "HeroWorld" server
#
# This script teleports the player to a ship's cabin.
# When they exit the cabin, they should appear wherever the ship is, rather than where it was.
#
# NEEDS WORK: Map is generated by the script itself, and given a unique name to avoid collision.
# However, this needs to be changed to be unique to each boat, rather than each activation.
#

import Crossfire
import random
import os.path
import CFDataBase


# Cabin size
size_x = 9
size_y = 9

# What floors are used.  Picked randomly from a list.  Formatted like ['object1', 'object2'].
floors = [ 'woodfloor' ]
# What floors are used.  Picked randomly from a list.
walls = [ 'timberwall' ]

# This is where the pointer to the ship is stored.
cabindict = Crossfire.GetPrivateDictionary()

def get_one(randobj):
	#Returns a random entry from an array (an entry in "floors" or "walls" up above, in this case).
	return randobj[ random.randint( 1, len(randobj) ) - 1 ]

def return_to_bed():
	#If the script breaks for some reason, return the player to their spawn point.
	flags = 0
	if (act.BedMap.find(Crossfire.PlayerDirectory()) != -1):
		# Unique map
		flags = 2
	dest = Crossfire.ReadyMap(act.BedMap, flags)
	if (dest == None):
		act.Message('The %s seems broken.'%l.Name)
		return

	act.Teleport(dest, act.BedX, act.BedY)

def do_back():
	#Teleports the player from the cabin to the current location of the ship.
	
	#Grabs the stored pointer to the ship cabin.  We need this to get the location, so we can teleport the player to the ship.
	shiploc = cabindict.get(act.Name)
	
	#act.Message('CabinDict: ' + str(cabindict[act.Name]))
	#act.Message('Ship x, y: ' + str(shiploc.X) + ', ' + str(shiploc.Y))
	
	#Grabs the ship information from the ship cabin's pointer.  The cabin's "Env" should be the ship's pointer.  "sm" is the map that the ship is in.
	x = shiploc.Env.X
	y = shiploc.Env.Y
	sm = shiploc.Env.Map.Path

	if x == '' or y == '' or sm == '':
		# If something's not getting grabbed, teleport the player back to their spawn point instead.
		act.Message('You feel a distorsion of reality!')
		return_to_bed()
		return
	
	#Tells the server to load the map (before teleporting the playing there)
	dest = Crossfire.ReadyMap(sm)
	
	if (dest == None):
		# Weeger thinks this can happen if we teleport from a random map.  Probably not an issue for us, but let's leave it in just in case.
		act.Message('The %s rattles. It can\'t take you back to your starting point.'%l.Name)
		return_to_bed()
		return

	act.Message('You exit the ship\'s cabin.')
	act.Teleport(dest, int(x), int(y))

def do_cabin():
	#This both creates the cabin and teleports the player to it.
	#TODO: Check to see if there's already a cabin for this ship, and if so then teleport the player there without creating a cabin.
	
	#l may not be necessary here - it gets the cabin door's pointer, but do we do anything with it?
	l = Crossfire.WhoAmI()
	#act is the player
	act = Crossfire.WhoIsActivator()
	
	# Map generation
	m = Crossfire.CreateMap(size_x, size_y)
	m.Path = os.path.join(Crossfire.ScriptName(), Crossfire.WhoIsActivator().Name)
	floor = get_one(floors)

	# First, let's put the floors
	for x in range(size_x):
		for y in range(size_y):
			fl = Crossfire.CreateObjectByName(floor)
			fl.Teleport(m, x, y)

	# Then the walls
	wall = get_one(walls)
	# Top left, the inex (invis_exit) is where we teleport the player first, so that Crossfire knows they're no longer attached to the boat.
	#If you're repurposing this code for something else, then you can get rid of the inex.
	inex = Crossfire.CreateObjectByName('invis_exit')
	inex.Slaying = m.Path
	inex.HP = size_x - 1
	inex.SP = 4
	inex.Teleport(m, 0, 0)
	w = Crossfire.CreateObjectByName(wall + '_2_2_2')
	w.Teleport(m, 0, 0)
	# Top right
	w = Crossfire.CreateObjectByName(wall + '_2_2_3')
	w.Teleport(m, size_x - 1, 0)
	# Bottom left
	w = Crossfire.CreateObjectByName(wall + '_2_2_1')
	w.Teleport(m, 0, size_y - 1)
	# Bottom right
	w = Crossfire.CreateObjectByName(wall + '_2_2_4')
	w.Teleport(m, size_x - 1, size_y - 1)
	# Top and bottom
	for x in range(size_x - 2):
		w = Crossfire.CreateObjectByName(wall + '_2_1_2')
		w.Teleport(m, x + 1, 0)
		w = Crossfire.CreateObjectByName(wall + '_2_1_2')
		w.Teleport(m, x + 1, size_y - 1)
	# Left and right
	for y in range(size_y - 2):
		if y == 3:#This is where we're putting the door.  Technically this should be halfway through instead of 3, since we let the cabin size be defined up top.  #TODO decide on one or the other?
			w = Crossfire.CreateObjectByName(wall + '_2_1_1')
			w.Teleport(m, 0, y + 1)
			cabexdoor = Crossfire.CreateObjectByName('oakdoor_1')
			cabexdoor.Teleport(m, size_x - 1, 4)
			cabexeva = Crossfire.CreateObjectByName('event_apply')#The script that we attach to the door, to return the player to the current boat location
			cabexeva.Title = 'Python'
			cabexeva.Slaying = '/python/items/ship-cabin.py'#Point this to the name of the file you are currently reading this comment in, unless you're doing something janky.
			cabexeva.InsertInto(cabexdoor)#attaching the script to the door
		else:#Actual left and right wall placement
			w = Crossfire.CreateObjectByName(wall + '_2_1_1')
			w.Teleport(m, 0, y + 1)
			w = Crossfire.CreateObjectByName(wall + '_2_1_1')
			w.Teleport(m, size_x - 1, y + 1)
			
	# Teleport
	
	cabindict[act.Name] = Crossfire.WhoAmI()#Stores a pointer to the ship's location, and associates it with the activating player.
	act.Message('You enter the ship\'s cabin.')
	act.Teleport(m, 0, 0)
	
act = Crossfire.WhoIsActivator()

if (act.Map.Path.find(Crossfire.ScriptName()) != -1):
	do_back()
else:
	do_cabin()

Crossfire.SetReturnValue(1)
