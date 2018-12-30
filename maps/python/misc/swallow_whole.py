import Crossfire
import random
#This script allows the purple worm to eat players/monsters
eatmap = '/planes/purpleworm'
eat_x = 6
eat_y = 6

worm = Crossfire.WhoAmI()
me = Crossfire.WhoIsOther()
r  = random.random()

if  (r <= 0.26):
    if (me.__class__ is Crossfire.Player):
	isplayer = 1
    elif (me.Alive == True):
	isplayer = 0
    else:
	isplayer = 2

    if isplayer == 1:
	me.Map.Print("\nYou are swallowed whole by the %s!" % (worm.Name))
	map = Crossfire.ReadyMap(eatmap)
	if map:
	    me.Teleport(map, eat_x, eat_y)
	else:
	    print "There is no eat map"

    elif isplayer == 0:
	me.Map.Print("\nThe %s is swallowed whole by the %s!" % (me.Name, worm.Name))

	while (me.Inventory is not None):
	    me.Inventory.InsertInto(worm)

	mexists = Crossfire.WhoIsOther()
	if mexists:
	    me.Remove()
	else:
	    worm.Map.Print('doesnt exist')
else:
    me.Map.Print("\nThe %s misses the %s" % (worm.Name, me.Name))
