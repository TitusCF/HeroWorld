import Crossfire
#This script turns players to stone
#Whenever a new player race is added please update the if/elif part of the script and make a new stoned face and stoned statue arch
stonedmap = '/planes/stoned'
stoned_x = 2
stoned_y = 2

#activator = Crossfire.WhoIsActivator()
me = Crossfire.WhoIsOther()
plarch = me.ArchName

smap = me.Map
sx = me.X
sy = me.Y
sname = me.Name
stype = 'creature'

if (me.__class__ is Crossfire.Player):
    isplayer = 1
else:
    isplayer = 0
    
if plarch == 'dwarf_player':
    statue = Crossfire.CreateObjectByName("dwarf_player_stoned")
elif plarch == 'elf_player':
    statue = Crossfire.CreateObjectByName("elf_player_stoned")
elif plarch == 'fenx_player':
    statue = Crossfire.CreateObjectByName("fenx_player_stoned")
elif plarch == 'fireborn_player':
    statue = Crossfire.CreateObjectByName("fireborn_player_stoned")
elif plarch == 'gnome_player':
    statue = Crossfire.CreateObjectByName("gnome_player_stoned")
elif plarch == 'halfling_player':
    statue = Crossfire.CreateObjectByName("halfling_player_stoned")
elif plarch == 'human_player':
    statue = Crossfire.CreateObjectByName("human_player_stoned")
elif plarch == 'northman_player':
    statue = Crossfire.CreateObjectByName("northman_player_stoned")
elif plarch == 'pl_dragon':
    statue = Crossfire.CreateObjectByName("pl_dragon_stoned")
elif plarch == 'pl_half_orc':
    statue = Crossfire.CreateObjectByName("pl_half_orc_stoned")
elif plarch == 'quetzalcoatl_player':
    statue = Crossfire.CreateObjectByName("quetzalcoatl_player_stoned")
elif plarch == 'serpentman_player':
    statue = Crossfire.CreateObjectByName("serpentman_player_stoned")
elif plarch == 'troll_player':
    statue = Crossfire.CreateObjectByName("troll_player_stoned")
elif plarch == 'wolf_player':
    statue = Crossfire.CreateObjectByName("wolf_player_stoned")
elif plarch == 'wraith_player':
    statue = Crossfire.CreateObjectByName("wraith_player_stoned")
elif plarch == 'golem':
    isplayer = 0
    statue = Crossfire.CreateObjectByName("stonegolem")
elif plarch == 'papergolem':
    isplayer = 0
    statue = Crossfire.CreateObjectByName("stonegolem")
elif plarch == 'strawgolem':
    isplayer = 0
    statue = Crossfire.CreateObjectByName("stonegolem")
elif plarch == 'ropegolem':
    isplayer = 0
    statue = Crossfire.CreateObjectByName("stonegolem")
elif plarch == 'platgolem':
    isplayer = 0
    statue = Crossfire.CreateObjectByName("stonegolem")
elif plarch == 'goldgolem':
    isplayer = 0
    statue = Crossfire.CreateObjectByName("stonegolem")
elif plarch == 'silvergolem':
    isplayer = 0
    statue = Crossfire.CreateObjectByName("stonegolem")
elif plarch == 'coppergolem':
    isplayer = 0
    statue = Crossfire.CreateObjectByName("stonegolem")
elif plarch == 'bronzegolem':
    isplayer = 0
    statue = Crossfire.CreateObjectByName("stonegolem")
elif plarch == 'giant':
    isplayer = 0
    statue = Crossfire.CreateObjectByName("stonegiant_onetile")
elif plarch == 'stonegolem':
    isplayer = 2
elif plarch == 'stonegiant_onetile':
    isplayer = 2
elif plarch == 'stonegiant':
    isplayer = 2
elif plarch == 'stonegiant_2':
    isplayer = 2
elif plarch == 'cockatrice':
    isplayer = 2
elif plarch == 'cockatrice_2':
    isplayer = 2
elif plarch == 'cockatrice_3':
    isplayer = 2
elif plarch == 'cockatrice_4':
    isplayer = 2
elif plarch == 'cockatrice_5':
    isplayer = 2
elif plarch == 'cockatrice_6':
    isplayer = 2			    
elif plarch == 'cockatrice_small':
    isplayer = 2
else:
    starch = plarch+'_stoned'
    statue = Crossfire.CreateObjectByName(starch)
    if statue.ArchName != starch:
	statue.Remove()
	if (me.Alive == True): 
	    statue = Crossfire.CreateObjectByName('generic_monster_stoned')
	else:
	    statue = Crossfire.CreateObjectByName('rubble')
	    stype = 'item'

if isplayer == 1:
    #Statue name should already be "statue" and setting the title should add "of foobar" nicely
    statue.Title = 'of %s, the level %s player' % (me.Name, me.Level)
    statue.InsertInto(me)
    me.Drop(statue)
    statue.Unique = True
    force = Crossfire.CreateObjectByName("force")
    force.Name = me.Name
    force.Speed = 0
    force.InsertInto(statue)
    statue.Map.Print("\nYou are blind\nYou cannot hear\nYou cannot move\nYou turn to stone")
    map = Crossfire.ReadyMap(stonedmap)
    if map:
	me.Teleport(map, stoned_x, stoned_y)
    else:
	print "There is no stoned map"

elif isplayer == 2:
    print "This creature cannot be turned to stone"
else:
    statue.Name = 'statue of a %s, the level %s %s' % (me.Name, me.Level, stype)
    statue.InsertInto(me)
    me.Drop(statue)
    sweight = me.Weight*10
    statue.Map.Print('The '+me.Name+' turned to stone.')
    
    while (me.Inventory is not None):
	me.Inventory.InsertInto(statue)
    
    mexists = Crossfire.WhoIsOther()
    if mexists:
	me.Remove()
    else:
	statue.Map.Print('doesnt exist')
	
    statue.Weight = sweight
    statue.Pickable = True
    #Doesn't work with many multi tile monsters for some reason
    
    
