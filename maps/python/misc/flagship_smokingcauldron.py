# Script for the Ship's Cabin functionality
#
# Written by James W. Bennett in 2020, based on Nicolas Weeger's "banquet.py" script from 2007
# Released as GPL
# Original concept/reason for writing was Titus', for their "HeroWorld" server
#
# This script teleports the player to a ship's cabin.
# When they exit the cabin, they should appear wherever the ship is, rather than where it was.
# 
# USING THIS SCRIPT:
# To use this script, there needs to be a writable map folder with a cabin template (see the 'template' variable).
# We're using /ship-cabins/, off of the maps folder.
# This script also expects an 'event_apply' event to be both on a unique ship and on a unique item called "cabin door" in that boat's inventory.
# Being "unique" ('unique 1' in the map editor) allows the ship to not reset its location and contents with each map reset.
#
# CHANGING THIS SCRIPT:
# 'tree5' is a placeholder non-exit item we use.  You can change it, but if you make it an exit then strange things might occur due to the 'unique 1' value.
# More importantly, the exit is set to coordinates 8, 4.  If you change the template map's exit tile, then you need to change these coordinates as well.
# Lastly, if you change the folder that the ship-cabins and serial are stored in, don't forget to change it everywhere.
#

import Crossfire
import os.path

# This is where the pointer to the ship is stored.
cabindict = Crossfire.GetPrivateDictionary()

# l is the item being activated, usually either the door to the cabin or the exit from the cabin
l = Crossfire.WhoAmI()
# act is the player
act = Crossfire.WhoIsActivator()

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
	# Teleports the player from the cabin to the current location of the ship.
	
	# Grabs the stored pointer to the ship cabin.  We need this to get the location, so we can teleport the player to the ship.
	shipserial = str(l.ReadKey('ship_serial'))
	shiploc = cabindict.get(shipserial)
	
	try:# This part fires if the ship is still in an active map
		# Grabs the ship information from the ship cabin's pointer.  The cabin's "Env" should be the ship's pointer.  "sm" is the map that the ship is in.
		x = shiploc.Env.X
		y = shiploc.Env.Y
		sm = shiploc.Env.Map.Path
	except (ReferenceError, AttributeError):# This part only fires if the ship the map was in got unloaded, in which case we refer to the last location that anybody applied the ship from.
		x = l.ReadKey('backup-loc_x')
		y = l.ReadKey('backup-loc_y')
		sm = l.ReadKey('backup-loc_map')
		
	if x == '' or y == '' or sm == '':
		# If something's not getting grabbed, teleport the player back to their spawn point instead.
		act.Message('You feel a distorsion of reality!')
		return_to_bed()
		return
	
	# Tells the server to load the map (before teleporting the playing there)
	dest = Crossfire.ReadyMap(sm)
	
	if (dest == None):
		# Weeger thinks this can happen if we teleport from a random map.  Probably not an issue for us, but let's leave it in just in case.
		act.Message('ERROR: Can\'t return to starting point.  Were you in a random map?  Returning to spawn point instead.')
		return_to_bed()
		return

	act.Message('You exit the ship\'s cabin.')
	act.Teleport(dest, int(x), int(y))
		
def do_cabin():
	#This both creates the cabin and teleports the player to it.
	if l.ReadKey('ship_serial') is not '':# Teleports the player in if the map's already made
		shipserial = str(l.ReadKey('ship_serial'))
		dest = Crossfire.ReadyMap('/ship-cabins/cabin-' + shipserial)
		
	else:# Creates the map before sending the player to it
		datadir = Crossfire.DataDirectory()
		mapdir = Crossfire.MapDirectory()
		
		# This stores an incrementing number that uniquely identifies the cabin map.  We will later write this number, as a key, to the cabin door.
		if not os.path.exists(str(datadir) + '/' + str(mapdir) + '/ship-cabins/ship-serial.txt'):# Create the file if it doesn't exist, and set the counter to 0.
			with open(str(datadir) + '/' + str(mapdir) + '/ship-cabins/ship-serial.txt', 'w') as shipfile:
				shipfile.write('0')
		with open(str(datadir) + '/' + str(mapdir) + '/ship-cabins/ship-serial.txt', 'r+') as shipfile:# Grab the current serial, then increment it because we have a new ship's cabin
			shipserial = shipfile.read()
			shipserial = int(shipserial)
			shipserial += 1
			shipserial = str(shipserial)
		with open(str(datadir) + '/' + str(mapdir) + '/ship-cabins/ship-serial.txt', 'w') as shipfile:# Overwrite the old file with the new serial
			shipfile.write(str(shipserial))
		
		template = (str(datadir) + '/' + str(mapdir) + '/ship-cabins/flagship_smokingcauldron')# Template for the cabin map.  We could have this randomly picked from a list of template maps, but right now we just have the one template.
		cabinfile = (str(datadir) + '/' + str(mapdir) + '/ship-cabins/cabin-' + shipserial)# Names our copy of the template map as ending in the serial number, like "cabin-7"
		
		if not os.path.exists(cabinfile):# If the map doesn't exist (it shouldn't), we create it.
			
			# First we look at the template, and grab all of it (but replace the word "template" with the shipserial
			with open(template, 'r') as tfile:
				tdata = (tfile.read().replace('template', str(shipserial)))
			# Now we write the new file
			with open(cabinfile, 'w') as cfile:
				cfile.write(tdata)
			
			dest = Crossfire.ReadyMap('/ship-cabins/cabin-' + shipserial)
			# Invisible exit, it decouples the player and the ship.  Should be at x,y = 0,0, and the player should enter the map at those coords.  HP and SP are x,y of where the inex will teleport the player afterward.
			inex = Crossfire.CreateObjectByName('invis_exit')
			inex.Slaying = dest.Path
			inex.HP = 27
			inex.SP = 7
			inex.Teleport(dest, 0, 0)
			# The cabin's exit door.
			cabexdoor = Crossfire.CreateObjectByName('tree5')# Avoiding making it an actual exit, because the "unique" flag does special things to exits (and we don't want that)
			cabexdoor.Unique = 1
			cabexdoor.Face = 'oakdoor_1.111'
			cabexdoor.Name = 'oak door'
			cabexdoor.Teleport(dest, 27, 7)
			cabexdoor.WriteKey('cabin_exit', '1', 1)
			cabexdoor.WriteKey('ship_serial', str(shipserial), 1)
			cabexeva = Crossfire.CreateObjectByName('event_apply')# The script that we attach to the door, to return the player to the current boat location
			cabexeva.Title = 'Python'
			cabexeva.Slaying = str(Crossfire.ScriptName()).replace((str(datadir) + '/' + str(mapdir)), '')# This should point to the path of this script file (filename included), relative to the map directory.
			cabexeva.InsertInto(cabexdoor)# Attaching the script to the door
			l.WriteKey('ship_serial', str(shipserial), 1)
			
		else:
			act.Message('ERROR: The cabin map file already exists, but the door didn\'t have a connector to it.  The contents of the /ship-cabins/ directory may need fixing.')
			# If the above ever happens, it's possible that the shipserial file got deleted or reset.
			# To fix that, set the number in the file to be equal to the highest suffix in that folder (so if /ship-cabins/cabin-42 exists, the contents of the serial should just be 42)
			# You may instead need to check the permissions on the folder, but if CF can't write to its own folders then you probably see many other errors.
		
	#Teleport the player
	act.Message('You enter the ship\'s cabin.')
	act.Teleport(dest, 0, 0)
	
	cabexdoor = dest.Check('tree5', (27, 7))# If you want to use a different placeholder item for the cabin exit, change all references to "tree5" to your new arch
	shipserial = str(cabexdoor.ReadKey('ship_serial'))
	cabindict[shipserial] = Crossfire.WhoAmI()# Stores a pointer to the ship's (external) door's location, and associates it with the internal cabin door
	cabexdoor.WriteKey('backup-loc_x', str(l.Env.X), 1)# Backs up the ship's location, similar to do_backup_loc()
	cabexdoor.WriteKey('backup-loc_y', str(l.Env.Y), 1)
	cabexdoor.WriteKey('backup-loc_map', str(l.Env.Map.Path), 1)
	
	
def do_backup_loc():
	#This is for backing up the location of the ship, so that the player can still exit the cabin if the outside map resets.
	if l.CheckInventory('cabin door').ReadKey('ship_serial') is not '':
		shipserial = l.CheckInventory('cabin door').ReadKey('ship_serial')
		dest = Crossfire.ReadyMap('/ship-cabins/cabin-' + shipserial)# We need to map to grab the door
		cabexdoor = dest.Check('tree5', (27, 7))
		cabexdoor.WriteKey('backup-loc_x', str(l.X), 1)
		cabexdoor.WriteKey('backup-loc_y', str(l.Y), 1)
		cabexdoor.WriteKey('backup-loc_map', str(l.Map.Path), 1)
		
if l.ReadKey('cabin_exit') is '1':
	do_back()
	Crossfire.SetReturnValue(1)
elif l.Type is 2:#Type 2 is transport
	Crossfire.SetReturnValue(0)# 0 causes the "apply" action to still happen to the ship, rather than be intercepted by the script
	do_backup_loc()
else:
	do_cabin()
	Crossfire.SetReturnValue(1)