import Crossfire
import shutil
import re
import random
#Buildable Building code by MikeeUSA (with help from Rednaxela) (C) GPL (v2 or later at your option)

templatedir = '/home/cfserver/cfservernew/share/crossfire/maps/templates/buildable/'
builtdir = '/home/cfserver/cfservernew/share/crossfire/maps/world_built/'
antarcticworld = 0 #By default we are not in the snow underworld.
aquiferworld = 0 #By default we are not in the eastern underworld.
interworld = 0 #By default we are not in the germanic underworld.
bottomworld = 0 #By default we are not in greece.
playergetkeys = 1 #Normal buildable buildings have locked areas, if not this will be set to 0 in the if-then.  
newmapcreated = 1 #Normally a new map is created
itemswaparchname = 'potato'
itemswapnumber = '10' #For farms, what nrof number will we look for to swap with another number
buildonprospect = 0 #Only when building mines
buildharbor = 0 #Only when building a harbor and when it is found to be the right tile

buildingtype = Crossfire.ScriptParameters()
if (buildingtype == 'test'):
    unique = 0
    #unique = 1
    #buildingtype = 'tob'
    #buildingtype = 'house'
    #buildingtype = 'temple'
    #buildingtype = 'shop'
    #buildingtype = 'well'
    #buildingtype = 'fountain'
    #buildingtype = 'farm'
    #buildingtype = 'prospect'
    #buildingtype = 'mine'
    #buildingtype = 'harbor'
    #buildingtype = 'guild'
    buildingtype = 'lamp'
else:
    unique = 1

if (buildingtype == 'prospect'):
    unique = 0
    newmapcreated = 0
elif (buildingtype == 'lamp'):
    newmapcreated = 0
     
me = Crossfire.WhoIsActivator()
plarch = me.ArchName

smap = '%s' % (me.Map.Path)
sx = '%s' % (me.X)
sy = '%s' % (me.Y)
sname = '%s' % (me.Name)

if (me.__class__ is Crossfire.Player):
    isplayer = 1
else:
    isplayer = 0

pattern = 'world_'
result = re.search(pattern, smap)
if result:
    pattern = 'bottomworld_'
    result = re.search(pattern, smap)
    if result:
        bottomworld = 1
    else:
        pattern = 'interworld_'
        result = re.search(pattern, smap)
        if result:
            interworld = 1
        else:
            pattern = 'aquiferworld_'
            result = re.search(pattern, smap)
            if result:
                aquiferworld = 1
            else:
                pattern = 'antarcticworld_'
                result = re.search(pattern, smap)
                if result:
                   antarcticworld  = 1
                    
else:
    isplayer = 0
    me.Map.Print('You must be outside to build.')
    
if isplayer == 1:
    Ob = me
    Ob = Ob.Below

    while Ob:
        if ((buildingtype == 'mine') and ((Ob.ArchName == 'prospecthole_coal')
        or (Ob.ArchName == 'prospecthole_copper')
        or (Ob.ArchName == 'prospecthole_gold')
        or (Ob.ArchName == 'prospecthole_iron')
        or (Ob.ArchName == 'prospecthole_lead')
        or (Ob.ArchName == 'prospecthole_nothing')
        or (Ob.ArchName == 'prospecthole_plat')
        or (Ob.ArchName == 'prospecthole_silver')
        or (Ob.ArchName == 'prospecthole_tin')
        or (Ob.ArchName == 'prospecthole_worthless_b')
        or (Ob.ArchName == 'prospecthole_worthless_g')
        or (Ob.ArchName == 'prospecthole_worthless_mixed')
        or (Ob.ArchName == 'prospecthole_worthless_r')
        or (Ob.ArchName == 'prospecthole_worthless_y')
        or (Ob.ArchName == 'prospecthole_zinc')
        or (Ob.ArchName == 'prospecthole_gem_agate')
        or (Ob.ArchName == 'prospecthole_gem_amethyst')
        or (Ob.ArchName == 'prospecthole_gem_aquamarine_gem')
        or (Ob.ArchName == 'prospecthole_gem_chalcedony')
        or (Ob.ArchName == 'prospecthole_gem_chrysoberyl')
        or (Ob.ArchName == 'prospecthole_gem_emerald')
        or (Ob.ArchName == 'prospecthole_gem_garnet')
        or (Ob.ArchName == 'prospecthole_gem_gem')
        or (Ob.ArchName == 'prospecthole_gem_jacinth')
        or (Ob.ArchName == 'prospecthole_gem_mithril')
        or (Ob.ArchName == 'prospecthole_gem_onyx')
        or (Ob.ArchName == 'prospecthole_gem_peridot')
        or (Ob.ArchName == 'prospecthole_gem_quartz')
        or (Ob.ArchName == 'prospecthole_gem_ruby')
        or (Ob.ArchName == 'prospecthole_gem_sapphire')
        or (Ob.ArchName == 'prospecthole_gem_topaz')
        or (Ob.ArchName == 'prospecthole_gem_tourmaline')
        or (Ob.ArchName == 'prospecthole_mineral_cinnabar')
        or (Ob.ArchName == 'prospecthole_mineral_dust_generic')
        or (Ob.ArchName == 'prospecthole_mineral_graphite')
        or (Ob.ArchName == 'prospecthole_mineral_gypsum')
        or (Ob.ArchName == 'prospecthole_mineral_phosphorus')
        or (Ob.ArchName == 'prospecthole_mineral_pyrite')
        or (Ob.ArchName == 'prospecthole_mineral_salt')
        or (Ob.ArchName == 'prospecthole_mineral_sulphur'))):
	    prospecttype = Ob.ArchName #so we know what type of mine to build
	    buildonprospect = 1
	    Ob.Remove() #Get rid of prospect hole
	    Ob = me
	    Ob = Ob.Below #Reset Ob
        elif ((buildingtype == 'harbor') and (Ob.ArchName == 'sea')):
            me.Map.Print('You prepare to build harbor.')
	    buildharbor = 1
        elif ((buildingtype == 'harbor') and (Ob.ArchName == 'shallow_sea')):
	    isplayer = 0
            me.Map.Print('The sea is too shallow here to build a harbor')
        elif ((buildingtype == 'harbor') and (Ob.ArchName == 'deep_sea')):
	    isplayer = 0
            me.Map.Print('The sea is too deep here to build a harbor')
        elif ((buildingtype == 'harbor') and (Ob.ArchName == 'deepsea')):
	    isplayer = 0
	    me.Map.Print('The sea is too deep here to build a harbor')
        elif (Ob.Type == 66):
            isplayer = 0
            me.Map.Print('You cannot build ontop of another building or exit.')
        elif (Ob.Type == 94):
            isplayer = 0
            me.Map.Print('You cannot build ontop of another building or exit.')
        elif (Ob.Type == 95):
            isplayer = 0
            me.Map.Print('You cannot build ontop of another building or exit.')
        elif (Ob.Type == 41):
            isplayer = 0
            me.Map.Print('You cannot build ontop of another building or exit.')
        elif (Ob.Type == 57):
            isplayer = 0
            me.Map.Print('You cannot build ontop of another building or exit.')
        elif (Ob.ArchName == 'dungeon_floor'):
            isplayer = 0
            me.Map.Print('You cannot build on a road or town square unless you want to die.')
        elif (Ob.ArchName == 'cobblestones2'):
            isplayer = 0
            me.Map.Print('You cannot build on a road or town square unless you want to die.')
        elif (Ob.ArchName == 'cobblestones'):
            isplayer = 0
            me.Map.Print('You cannot build on a road or town square unless you want to die.')
        elif (Ob.ArchName == 'flagstone'):
            isplayer = 0
            me.Map.Print('You cannot build on a road or town square unless you want to die.')
        elif (Ob.ArchName == 'shallow_sea'):
            isplayer = 0
            me.Map.Print('You cannot build into the sea.')
        elif (Ob.ArchName == 'sea'):
            isplayer = 0
            me.Map.Print('You cannot build into the sea.')
        elif (Ob.ArchName == 'deep_sea'):
            isplayer = 0
            me.Map.Print('You cannot build into the sea.')
        elif (Ob.ArchName == 'deepsea'):
            isplayer = 0
            me.Map.Print('You cannot build into the sea.')
        elif (Ob.ArchName == 'blood_sea'):
            isplayer = 0
            me.Map.Print('You cannot build atop a sea of bloods: try creating land by sinking a mountain of heads.')
        elif (Ob.ArchName == 'pstone_2'):
            isplayer = 0
            me.Map.Print('You cannot build on a path.')
        elif (Ob.ArchName == 'pstone_1'):
            isplayer = 0
            me.Map.Print('You cannot build on a path.')
        elif (Ob.ArchName == 'farmland'):
            isplayer = 0
            me.Map.Print('You cannot build on farmland.')
        elif (Ob.ArchName == 'fountain'):
            isplayer = 0
            me.Map.Print('You cannot build on a fountain!')
        elif (Ob.Floor != True):
            isplayer = 0
            me.Map.Print('Debirs must be cleared from land before building: use a fire spell or ability to clear the way.')
            
        Ob = Ob.Below

if (buildingtype == 'house'):
    region = '%s' % (me.Map.Region.Name)
    
    entertox = 8
    entertoy = 14
    
    if (bottomworld == 1):
        housearch = 'house_1_greco'
        housemap = 'house_greco'
        housename = 'house'
    elif ((interworld == 1) 
    and ((plarch == 'human_player')
    or (plarch == 'northman_player')
    or (plarch == 'gnome_player')
    or (plarch == 'halfling_player'))):
        housearch = 'barrack_small_goth'
        housemap = 'house_gotisch'
        housename = 'haus'
    elif ((region == 'navar')
    or (region == 'navarempire')
    or (region == 'celvear')
    or (region == 'lostwages')):
        housearch = 's_shop2'
        housemap = 'house_navar'
        housename = 'house'
    elif ((region == 'scorn')
    or (region == 'scorncounty')):
        housearch = 'house_small'
        housemap = 'house_scorn'
        housename = 'house'
    elif (region == 'darcap'):
        housearch = 'shrine_northwest'
        housemap = 'house_darcap'
        housename = 'abode'
    elif (region == 'gotisch'):
        housearch = 'barrack_small_goth'
        housemap = 'house_gotisch'
        housename = 'haus'
    elif ((region == 'azumauindo') or (aquiferworld == 1)):
        housearch = 'house1_east'
        housemap = 'house_azumauindo'
        housename = 'sumai'
    elif ((region == 'brest')
    or (region == 'brittany')):
        housearch = 'house_2'
        housemap = 'house_brest'
        housename = 'house'
    elif ((region == 'antarctica') or (antarcticworld == 1)):
        housearch = 's_house_2'
        housemap = 'house_antarctica'
        housename = 'house'
    elif ((region == 'wilderness')
    or ((interworld == 1)
    and ((plarch != 'human_player')
    and (plarch != 'northman_player')
    and (plarch != 'gnome_player')
    and (plarch != 'halfling_player')))):
	if plarch == 'dwarf_player':
	    housearch = 'mountain_cave2'
	    housemap = 'house_cave_dwarf'
	    housename = 'dwarvish cave'
	elif plarch == 'elf_player':
	    housearch = 'treehouse'
	    housemap = 'house_tree'
	    housename = 'treehouse'
	elif plarch == 'fenx_player':
	    housearch = 'house_2'
	    housemap = 'house_standard'
	    housename = 'house'
	elif plarch == 'fireborn_player':
	    housearch = 'burning_house_1'
	    housemap = 'house_burn'
	    housename = 'house'
	elif plarch == 'gnome_player':
	    housearch = 'house_2'
	    housemap = 'house_standard'
	    housename = 'house'
	elif plarch == 'halfling_player':
	    housearch = 'hut'
	    housemap = 'house_hut'
	    housename = 'hut'
	elif plarch == 'human_player':
	    housearch = 'woodhouse'
	    housemap = 'house_older'
	    housename = 'house'
	elif plarch == 'northman_player':
	    housearch = 'house_2'
	    housemap = 'house_standard'
	    housename = 'house'
	elif plarch == 'pl_dragon':
	    housearch = 'mountain_cave'
	    housemap = 'house_cave'
	    housename = 'cave'
	elif plarch == 'pl_half_orc':
	    housearch = 'hut'
	    housemap = 'house_hut'
	    housename = 'hut'
	elif plarch == 'quetzalcoatl_player':
	    housearch = 'mountain_cave'
	    housemap = 'house_cave'
	    housename = 'cave'
	elif plarch == 'serpentman_player':
	    housearch = 'hut'
	    housemap = 'house_hut'
	    housename = 'hut'
	elif plarch == 'troll_player':
	    housearch = 'hut'
	    housemap = 'house_hut'
	    housename = 'hut'
	elif plarch == 'wolf_player':
	    housearch = 'mountain_cave'
	    housemap = 'house_cave'
	    housename = 'cave'
	elif plarch == 'wraith_player':
	    housearch = 'small_temple'
	    housemap = 'house_undead'
	    housename = 'tomb'
	else:								
	    housearch = 'woodhouse'
	    housemap = 'house_older'
	    housename = 'house'
    else:
        housearch = 'woodhouse'
	housemap = 'house_older'
	housename = 'house'
elif (buildingtype == 'tob'):
    region = '%s' % (me.Map.Region.Name)
    
    entertox = 7
    entertoy = 14
    
    if (bottomworld == 1):
        housearch = 'tower_tob_greco'
	housemap = 'tob_greco'
	housename = 'tower'
    elif ((region == 'navar')
    or (region == 'navarempire')
    or (region == 'celvear')
    or (region == 'lostwages')):
	housearch = 'tower_tob_west'
	housemap = 'tob_navar'
	housename = 'tower'
    elif ((region == 'scorn')
    or (region == 'scorncounty')):
	housearch = 'tower_tob'
	housemap = 'tob_scorn'
	housename = 'tower'    
    elif (region == 'darcap'):
	housearch = 'tower_tob_northwest'
	housemap = 'tob_darcap'
	housename = 'tower'
    elif ((region == 'gotisch') or (interworld == 1)):
	housearch = 'tower_tob_goth'
	housemap = 'tob_gotisch'
	housename = 'aufsatz'
    elif ((region == 'azumauindo') or (aquiferworld == 1)):
	entertox = 8
	entertoy = 13
	housearch = 'tower_tob_east'
	housemap = 'tob_azumauindo'
	housename = 'tou'
    elif ((region == 'brest')
    or (region == 'brittany')):
	housearch = 'tower_tob_fant'
	housemap = 'tob_brest'
	housename = 'tower'
    elif ((region == 'antarctica') or (antarcticworld == 1)):
    	housearch = 's_tower_tob'
	housemap = 'tob_antarctica'
	housename = 'tower' 
    elif (region == 'wilderness'):
	housearch = 'tower_tob_brown'
	housemap = 'tob_brown'
	housename = 'tower'
    else:
	housearch = 'tower_tob_brown'
	housemap = 'tob_brown'
	housename = 'tower'
elif (buildingtype == 'temple'):
    region = '%s' % (me.Map.Region.Name)
    
    entertox = 8
    entertoy = 14
    
    if (bottomworld == 1):
        housearch = 'small_temple2'
	housemap = 'temple_standard'
	housename = 'temple'
    elif ((region == 'navar')
    or (region == 'navarempire')
    or (region == 'celvear')
    or (region == 'lostwages')):
	housearch = 'small_temple2_west'
	housemap = 'temple_navar'
	housename = 'temple'
    elif ((region == 'gotisch') or (interworld == 1)):
	housearch = 'small_temple'
	housemap = 'temple_gotisch'
	housename = 'Bugel'
    elif ((region == 'azumauindo') or (aquiferworld == 1)):
	housearch = 'house2_east'
	housemap = 'temple_azumauindo'
	housename = 'ichidou'
    else:
	housearch = 'small_temple2'
	housemap = 'temple_standard'
	housename = 'temple'
elif (buildingtype == 'shop'):
    region = '%s' % (me.Map.Region.Name)
    
    entertox = 9
    entertoy = 14
    
    if (bottomworld == 1):
        housearch = 'r_house2'
	housemap = 'shop_greco'
	housename = 'shop'
    elif ((region == 'navar')
    or (region == 'navarempire')
    or (region == 'celvear')
    or (region == 'lostwages')):
	housearch = 's_shop1'
	housemap = 'shop_navar'
	housename = 'shop'
    elif ((region == 'scorn')
    or (region == 'scorncounty')):
	housearch = 'minihouse_shop'
	housemap = 'shop_scorn'
	housename = 'shop'
    elif ((region == 'gotisch') or (interworld == 1)):
	housearch = 'barrack_small_goth_shop'
	housemap = 'shop_gotisch'
	housename = 'Geschaft'
    elif ((region == 'azumauindo') or (aquiferworld == 1)):
	housearch = 'smallmarket1_east'
	housemap = 'shop_azumauindo'
	housename = 'mise'
    elif (region == 'darcap'):
	housearch = 'smallmarket1_northwest'
	housemap = 'shop_darcap'
	housename = 'mise'
    elif ((region == 'brest')
    or (region == 'brittany')):
	housearch = 'tavern_fant_shop'
	housemap = 'shop_brest'
	housename = 'shop'
    elif ((region == 'antarctica') or (antarcticworld == 1)):
    	housearch = 's_woodhouse_shop'
	housemap = 'shop_antarctica'
	housename = 'shop'
    else:
	housearch = 'woodhouse_shop'
	housemap = 'shop_standard'
	housename = 'shop'
elif (buildingtype == 'well'):
    playergetkeys = 0
    region = '%s' % (me.Map.Region.Name)

    entertox = 9
    entertoy = 10

    Ob = me
    Ob = Ob.Below

    if ((region == 'antarctica') or (antarcticworld == 1)):
    	randnum = random.randrange(1, 7)
	# print randnum
	if (randnum == 6):
        	housearch = 'wellofoil'
        	housemap = 'well_oilsnow'
        	housename = 'oil well'
	else:
                housearch = 'well'
                housemap = 'well_snow'
                housename = 'cold well'
    elif ((Ob.ArchName == 'small_stones')
    or (Ob.ArchName == 'steppe')
    or (Ob.ArchName == 'pstone_1')
    or (Ob.ArchName == 'pstone_2')
    or (Ob.ArchName == 'medium_stones')
    or (Ob.ArchName == 'large_stones')
    or (Ob.ArchName == 'pstone_3')
    or (Ob.ArchName == 'pstone_4')
    or (Ob.ArchName == 'pstone_5')
    or (Ob.ArchName == 'crater')
    or (Ob.ArchName == 'lostlabdesert')
    or (Ob.ArchName == 'lostlabdesert2')
    or (Ob.ArchName == 'lostlabdesert3')
    or (Ob.ArchName == 'lostlabdesert4')
    or (Ob.ArchName == 'lostlabdesert5')
    or (Ob.ArchName == 'lostlabgreysand')
    or (Ob.ArchName == 'steppelight')):
        housearch = 'well'
        housemap = 'well_dry'
        housename = 'dry well'
    elif ((Ob.ArchName == 'desert')
    or (Ob.ArchName == 'dunes')):
    	if ((region == 'navar')
	or (region == 'navarempire')
    	or (region == 'celvear')
    	or (region == 'lostwages')):
		randnum = random.randrange(1, 5)
	elif (region == 'azumauindo'):
		randnum = random.randrange(1, 10)
	elif (region == 'gotisch'):
		randnum = random.randrange(1, 40)
	elif (region == 'darcap'):
		randnum = random.randrange(1, 30)
	elif ((region == 'brest')
    	or (region == 'brittany')
    	or (region == 'scorn')
    	or (region == 'scorncounty')):
		randnum = random.randrange(1, 200)
	elif (region == 'wilderness'):
		randnum = random.randrange(1, 20)
	else:
		randnum = random.randrange(1, 15)
		
	# print randnum
	
	if (randnum == 4):
        	housearch = 'wellofoil'
        	housemap = 'well_oildry'
        	housename = 'oil well'
	else:
                housearch = 'well'
                housemap = 'well_dry'
                housename = 'dry well'
    elif ((Ob.ArchName == 'beach')
    or (Ob.ArchName == 'brush')
    or (Ob.ArchName == 'cyanbrush')
    or (Ob.ArchName == 'darkforest')
    or (Ob.ArchName == 'jungle_1')
    or (Ob.ArchName == 'jungle_2')
    or (Ob.ArchName == 'grass')
    or (Ob.ArchName == 'grass_only')
    or (Ob.ArchName == 'bosgrass')
    or (Ob.ArchName == 'bosgrass2')
    or (Ob.ArchName == 'bosgrass3')
    or (Ob.ArchName == 'bosgrass4')
    or (Ob.ArchName == 'bosgrass5')
    or (Ob.ArchName == 'bosgrass6')
    or (Ob.ArchName == 'bosgrass7')
    or (Ob.ArchName == 'bosgrass8')
    or (Ob.ArchName == 'bosgrass9')
    or (Ob.ArchName == 'grassdark')
    or (Ob.ArchName == 'grassmedium')):
        housearch = 'well'
        housemap = 'well_mossy'
        housename = 'well'
    elif ((Ob.ArchName == 'swamp')
    or (Ob.ArchName == 'marsh')
    or (Ob.ArchName == 'deep_swamp')):
        housearch = 'well'
        housemap = 'well_swamp'
        housename = 'corrupted well'
    elif ((Ob.ArchName == 'mountain')
    or (Ob.ArchName == 'mountain2')
    or (Ob.ArchName == 'mountain_2')
    or (Ob.ArchName == 'mountain3')):
        housearch = 'well'
        housemap = 'well_mountain'
        housename = 'well'
    elif ((Ob.ArchName == 'mountain4')
    or (Ob.ArchName == 'mountain5')
    or (Ob.ArchName == 's_mountain')):
        housearch = 'well'
        housemap = 'well_snowmountain'
        housename = 'cold well'
    elif ((Ob.ArchName == 'istone')
    or (Ob.ArchName == 'ipond')
    or (Ob.ArchName == 'nasty_ice')
    or (Ob.ArchName == 'evil_ice')
    or (Ob.ArchName == 'drifts')
    or (Ob.ArchName == 'cmarsh')
    or (Ob.ArchName == 'snow')
    or (Ob.ArchName == 'snow2')
    or (Ob.ArchName == 'snow3')
    or (Ob.ArchName == 'snow4')
    or (Ob.ArchName == 'snow5')
    or (Ob.ArchName == 'glacier')
    or (Ob.ArchName == 'ice')
    or (Ob.ArchName == 'ice2')):
        housearch = 'well'
        housemap = 'well_snow'
        housename = 'cold well'
    else:
    	housearch = 'well'
        housemap = 'well_watered'
        housename = 'well'
elif (buildingtype == 'fountain'):
    playergetkeys = 0
    region = '%s' % (me.Map.Region.Name)

    entertox = 5
    entertoy = 9

    Ob = me
    Ob = Ob.Below
    
    randnum = random.randrange(1, 1500)
    
    # print randnum
    
    if (randnum == 602):
    	housearch = 'quicksilver_fountain_exit'
    	housemap = 'fountain_quicksilver'
    	housename = 'quicksilver fountain'
    elif (bottomworld == 1):
        housearch = 'cyan_fountain_exit'
        housemap = 'fountain_cyan'
        housename = 'mineral water fountain'
    elif ((region == 'antarctica') or (antarcticworld == 1)):
    	randnum = random.randrange(1, 17)
	# print randnum
	if (randnum == 6):
        	housearch = 'oil_fountain_exit'
        	housemap = 'fountain_oil'
        	housename = 'oil fountain'
	else:
                housearch = 'snow_fountain_exit'
                housemap = 'fountain_cold'
                housename = 'cold fountain'
    elif ((region == 'gotisch') or (interworld == 1)):
    	randnum = random.randrange(1, 4)
	# print randnum
	if (randnum == 3):
        	housearch = 'burning_fountain_exit'
        	housemap = 'fountain_blood'
        	housename = 'blood fountain'
	else:
                housearch = 'fountain_exit'
                housemap = 'fountain_standard'
                housename = 'fountain'		
    elif ((region == 'brest')
    or (region == 'brittany')
    or (region == 'scorn')
    or (region == 'scorncounty')):
    	housearch = 'fountain_exit'
        housemap = 'fountain_standard'
        housename = 'fountain'	
    elif ((Ob.ArchName == 'small_stones')
    or (Ob.ArchName == 'steppe')
    or (Ob.ArchName == 'pstone_1')
    or (Ob.ArchName == 'pstone_2')
    or (Ob.ArchName == 'medium_stones')
    or (Ob.ArchName == 'large_stones')
    or (Ob.ArchName == 'pstone_3')
    or (Ob.ArchName == 'pstone_4')
    or (Ob.ArchName == 'pstone_5')
    or (Ob.ArchName == 'crater')
    or (Ob.ArchName == 'lostlabdesert')
    or (Ob.ArchName == 'lostlabdesert2')
    or (Ob.ArchName == 'lostlabdesert3')
    or (Ob.ArchName == 'lostlabdesert4')
    or (Ob.ArchName == 'lostlabdesert5')
    or (Ob.ArchName == 'lostlabgreysand')
    or (Ob.ArchName == 'steppelight')):
        housearch = 'cyan_fountain_exit'
        housemap = 'fountain_cyan'
        housename = 'mineral water fountain'
    elif ((Ob.ArchName == 'desert')
    or (Ob.ArchName == 'dunes')):
    	if ((region == 'navar')
	or (region == 'navarempire')
    	or (region == 'celvear')
    	or (region == 'lostwages')):
		randnum = random.randrange(1, 10)
	elif (region == 'azumauindo'):
		randnum = random.randrange(1, 20)
	elif (region == 'darcap'):
		randnum = random.randrange(1, 60)
	elif (region == 'wilderness'):
		randnum = random.randrange(1, 40)
	else:
		randnum = random.randrange(1, 30)
		
	# print randnum
	
	if (randnum == 4):
        	housearch = 'oil_fountain_exit'
        	housemap = 'fountain_oil'
        	housename = 'oil fountain'
	else:
                housearch = 'cyan_fountain_exit'
        	housemap = 'fountain_cyan'
        	housename = 'mineral water fountain'
    elif ((Ob.ArchName == 'mountain4')
    or (Ob.ArchName == 'mountain5')
    or (Ob.ArchName == 's_mountain') 
    or (Ob.ArchName == 'istone')
    or (Ob.ArchName == 'ipond')
    or (Ob.ArchName == 'nasty_ice')
    or (Ob.ArchName == 'evil_ice')
    or (Ob.ArchName == 'drifts')
    or (Ob.ArchName == 'cmarsh')
    or (Ob.ArchName == 'snow')
    or (Ob.ArchName == 'snow2')
    or (Ob.ArchName == 'snow3')
    or (Ob.ArchName == 'snow4')
    or (Ob.ArchName == 'snow5')
    or (Ob.ArchName == 'glacier')
    or (Ob.ArchName == 'ice')
    or (Ob.ArchName == 'ice2')):
        housearch = 'snow_fountain_exit'
        housemap = 'fountain_cold'
        housename = 'cold fountain'
    else:
    	housearch = 'fountain_exit'
        housemap = 'fountain_standard'
        housename = 'fountain'
elif (buildingtype == 'farm'):
    playergetkeys = 0
    region = '%s' % (me.Map.Region.Name)

    entertox = 7
    entertoy = 10

    Ob = me
    Ob = Ob.Below
    
    if ((region == 'antarctica') or (antarcticworld == 1)):
        housearch = 'farmlandoverlaycoldexit'
        housemap = 'farmland_cold'
        housename = 'snowfield'
	itemswaparchname = 'icecube'
	itemswapnumber = '1'
    elif ((Ob.ArchName == 'small_stones')
    or (Ob.ArchName == 'steppe')
    or (Ob.ArchName == 'pstone_1')
    or (Ob.ArchName == 'pstone_2')
    or (Ob.ArchName == 'medium_stones')
    or (Ob.ArchName == 'large_stones')
    or (Ob.ArchName == 'pstone_3')
    or (Ob.ArchName == 'pstone_4')
    or (Ob.ArchName == 'pstone_5')
    or (Ob.ArchName == 'crater')
    or (Ob.ArchName == 'lostlabdesert')
    or (Ob.ArchName == 'lostlabdesert2')
    or (Ob.ArchName == 'lostlabdesert3')
    or (Ob.ArchName == 'lostlabdesert4')
    or (Ob.ArchName == 'lostlabdesert5')
    or (Ob.ArchName == 'lostlabgreysand')
    or (Ob.ArchName == 'steppelight')):
        housearch = 'farmlandoverlayexit'
        housemap = 'farmland_standard'
        housename = 'pipeweed farm'
	itemswaparchname = 'pipeweed'
	itemswapnumber = '5'
    elif ((Ob.ArchName == 'desert')
    or (Ob.ArchName == 'dunes')):
        housearch = 'farmlandoverlaydesertexit'
        housemap = 'farmland_desert'
        housename = 'farm of thorns'
	itemswaparchname = 'thorns'
	itemswapnumber = '1'     
    elif (Ob.ArchName == 'beach'):
	housearch = 'farmlandoverlayexit'
        housemap = 'farmland_standard'
        housename = 'mint farm'
	itemswaparchname = 'mint'
	itemswapnumber = '3'
    elif ((Ob.ArchName == 'jungle_1')
    or (Ob.ArchName == 'jungle_2')):
    	housearch = 'farmlandoverlayexit'
        housemap = 'farmland_standard'

	randnum = random.randrange(1, 4)
	if (randnum == 3):
		housename = 'pineapple field'
		itemswaparchname = 'pineapple'
		itemswapnumber = '10'
	else:
    		housename = 'bannana farm'
		itemswaparchname = 'bannana'
		itemswapnumber = '10'
    elif (Ob.ArchName == 'darkforest'):
	housearch = 'farmlandoverlayexit'
        housemap = 'farmland_standard'
        housename = 'clover farm'
	itemswaparchname = 'clover'
	itemswapnumber = '3'
    elif ((Ob.ArchName == 'brush')
    or (Ob.ArchName == 'cyanbrush')
    or (Ob.ArchName == 'grass')
    or (Ob.ArchName == 'grass_only')
    or (Ob.ArchName == 'bosgrass')
    or (Ob.ArchName == 'bosgrass2')
    or (Ob.ArchName == 'bosgrass3')
    or (Ob.ArchName == 'bosgrass4')
    or (Ob.ArchName == 'bosgrass5')
    or (Ob.ArchName == 'bosgrass6')
    or (Ob.ArchName == 'bosgrass7')
    or (Ob.ArchName == 'bosgrass8')
    or (Ob.ArchName == 'bosgrass9')
    or (Ob.ArchName == 'grassdark')
    or (Ob.ArchName == 'grassmedium')):
        if (bottomworld == 1):
        	housearch = 'farmlandoverlayexit'
        	housemap = 'farmland_standard'
        	housename = 'vineyard'
		itemswaparchname = 'grape'
		itemswapnumber = '100'
        elif ((region == 'azumauindo') or (aquiferworld == 1)):
        	housearch = 'farmlandoverlayexit'
        	housemap = 'farmland_standard'
        	housename = 'cherry orchard'
		itemswaparchname = 'cherry'
		itemswapnumber = '30'
        elif (region == 'darcap'):
        	housearch = 'farmlandoverlayexit'
        	housemap = 'farmland_standard'
        	housename = 'lemon orchard'
		itemswaparchname = 'lemon'
		itemswapnumber = '30'
        elif ((region == 'navar')
        or (region == 'navarempire')
        or (region == 'celvear')
        or (region == 'lostwages')):
        	housearch = 'farmlandoverlayexit'
		housemap = 'farmland_standard'
        	housename = 'orange orchard'
		itemswaparchname = 'orange'
		itemswapnumber = '30'
        elif ((region == 'scorn')
        or (region == 'scorncounty')):
        	housearch = 'farmlandoverlayexit'
        	housemap = 'farmland_standard'
        	housename = 'carrot farm'
		itemswaparchname = 'carrot'
		itemswapnumber = '30'
        elif ((region == 'gotisch') or (interworld == 1)):
        	housearch = 'farmlandoverlayexit'
        	housemap = 'farmland_standard'
        	housename = 'apple orchard'
		itemswaparchname = 'apple'
		itemswapnumber = '30'
        elif ((region == 'brest')
        or (region == 'brittany')):
        	housearch = 'farmlandoverlayexit'
        	housemap = 'farmland_standard'
        	housename = 'pear orchard'
		itemswaparchname = 'pear'
		itemswapnumber = '30'
        else:
        	housearch = 'farmlandoverlayexit'
        	housemap = 'farmland_standard'
        	housename = 'potato farm'
		itemswaparchname = 'potato'
		itemswapnumber = '15'
    elif ((Ob.ArchName == 'swamp')
    or (Ob.ArchName == 'marsh')
    or (Ob.ArchName == 'deep_swamp')):
        housearch = 'farmlandoverlayexit'
        housemap = 'farmland_standard'
        housename = 'corrupted farm'
	itemswaparchname = 'mushroom_1'
	itemswapnumber = '10'
    elif ((Ob.ArchName == 'mountain')
    or (Ob.ArchName == 'mountain2')
    or (Ob.ArchName == 'mountain_2')
    or (Ob.ArchName == 'mountain3')):
        housearch = 'farmlandoverlayexit'
        housemap = 'farmland_standard'
        housename = 'onion farm'
	itemswaparchname = 'onion'
	itemswapnumber = '4'
    elif ((Ob.ArchName == 'mountain4')
    or (Ob.ArchName == 'mountain5')
    or (Ob.ArchName == 's_mountain')
    or (Ob.ArchName == 'istone')
    or (Ob.ArchName == 'ipond')
    or (Ob.ArchName == 'nasty_ice')
    or (Ob.ArchName == 'evil_ice')
    or (Ob.ArchName == 'drifts')
    or (Ob.ArchName == 'cmarsh')
    or (Ob.ArchName == 'snow')
    or (Ob.ArchName == 'snow2')
    or (Ob.ArchName == 'snow3')
    or (Ob.ArchName == 'snow4')
    or (Ob.ArchName == 'snow5')
    or (Ob.ArchName == 'glacier')
    or (Ob.ArchName == 'ice')
    or (Ob.ArchName == 'ice2')):
        housearch = 'farmlandoverlaycoldexit'
        housemap = 'farmland_cold'
        housename = 'snowfield'
	itemswaparchname = 'icecube'
	itemswapnumber = '1'
    else:
    	housearch = 'farmlandoverlayexit'
        housemap = 'farmland_standard'
        housename = 'potato farm'
	itemswaparchname = 'potato'
	itemswapnumber = '4'
elif (buildingtype == 'prospect') or (buildingtype == 'mine'):	
	region = '%s' % (me.Map.Region.Name)
	Ob = me
	Ob = Ob.Below
	
	if (buildonprospect == 1):
		me.Map.Print('Preparing prospect for mine entrance')
		#prospecttype stays as it is
	else:
		#randomness
		#standard randomness
		
		plat_range = 2200000
		gold_range  = 220000
		silver_range = 22000
		copper_range =  4400
		iron_range =    2200
		lead_range =    1100
		zinc_range =     800
		tin_range =      500
		coal_range =     100
		worthless_b_range = 10
		worthless_g_range = 30
		worthless_r_range = 20
		worthless_y_range = 50
		worthless_mixed_range = 20
		nothing_range = 1
		
		gem_mithril_range = 2100000
		gem_gem_range =     1800000
		gem_ruby_range =    1100000
		gem_emerald_range =  800000
		gem_jacinth_range =  500000
		gem_sapphire_range = 300000
		gem_amethyst_range = 100000
		gem_topaz_range =     70000
		gem_peridot_range =   50000
		gem_aquamarine_gem_range = 30000
		gem_garnet_range = 20000
		gem_chrysoberyl_range = 18000
		gem_onyx_range =     10000
		gem_tourmaline_range = 5000
		gem_agate_range = 1000
		gem_chalcedony_range = 500
		gem_quartz_range = 100
		
		mineral_dust_generic_range = 5
		mineral_graphite_range = 10
		mineral_pyrite_range = 40
		mineral_gypsum_range = 50
		mineral_salt_range = 60
		mineral_sulphur_range = 100
		mineral_phosphorus_range = 400
		mineral_cinnabar_range = 500
		
		if (region == 'antarctica'):
			prospecttype = 'prospecthole_nothing'
			
			mineral_salt_range = 20
			gem_chalcedony_range = 200
			gem_aquamarine_gem_range = 10000
			gem_sapphire_range = 100000
			
			randnum = random.randrange(1, mineral_salt_range)
			# print randnum
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_salt'
				
			randnum = random.randrange(1, gem_chalcedony_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_chalcedony'
				
			randnum = random.randrange(1, gem_aquamarine_gem_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_aquamarine_gem'
				
			randnum = random.randrange(1, gem_sapphire_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_sapphire'
		
		elif ((Ob.ArchName == 'small_stones')
		or (Ob.ArchName == 'steppe')
		or (Ob.ArchName == 'pstone_1')
		or (Ob.ArchName == 'pstone_2')
		or (Ob.ArchName == 'medium_stones')
		or (Ob.ArchName == 'large_stones')
		or (Ob.ArchName == 'pstone_3')
		or (Ob.ArchName == 'pstone_4')
		or (Ob.ArchName == 'pstone_5')
		or (Ob.ArchName == 'crater')
		or (Ob.ArchName == 'lostlabdesert')
		or (Ob.ArchName == 'lostlabdesert2')
		or (Ob.ArchName == 'lostlabdesert3')
		or (Ob.ArchName == 'lostlabdesert4')
		or (Ob.ArchName == 'lostlabdesert5')
		or (Ob.ArchName == 'lostlabgreysand')
		or (Ob.ArchName == 'steppelight')):
			prospecttype = 'prospecthole_nothing'
			
			if ((region == 'navar')
			or (region == 'navarempire')
			or (region == 'celvear')
			or (region == 'lostwages')):
				lead_range =     600
				zinc_range =     600
				tin_range =      100
			elif (region == 'gotisch'):
				iron_range =     500
			elif ((region == 'scorn')
			or (region == 'scorncounty')):
				mineral_salt_range = 20
			
			randnum = random.randrange(1, mineral_dust_generic_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_dust_generic'
			
			randnum = random.randrange(1, mineral_graphite_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_graphite'
				
			randnum = random.randrange(1, worthless_b_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_worthless_b'
			randnum = random.randrange(1, worthless_g_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_worthless_g'
			randnum = random.randrange(1, worthless_r_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_worthless_r'
	
			randnum = random.randrange(1, mineral_pyrite_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_pyrite'
			randnum = random.randrange(1, mineral_gypsum_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_gypsum'
			randnum = random.randrange(1, mineral_salt_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_salt'
				
			randnum = random.randrange(1, tin_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_tin'
			randnum = random.randrange(1, zinc_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_zinc'
			randnum = random.randrange(1, lead_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_lead'
			randnum = random.randrange(1, iron_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_iron'
			randnum = random.randrange(1, copper_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_copper'
				
		elif ((Ob.ArchName == 'desert')
		or (Ob.ArchName == 'dunes')):
			prospecttype = 'prospecthole_nothing'
			
			if ((region == 'navar')
			or (region == 'navarempire')
			or (region == 'celvear')
			or (region == 'lostwages')):
				lead_range =     400
				zinc_range =     500
				tin_range =      10
			
			randnum = random.randrange(1, worthless_b_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_worthless_b'
			randnum = random.randrange(1, tin_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_tin'
			randnum = random.randrange(1, zinc_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_zinc'
			randnum = random.randrange(1, lead_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_lead'
		elif (Ob.ArchName == 'beach'):
			prospecttype = 'prospecthole_nothing'
			mineral_salt_range = 20
			gem_agate_range = 500
			gem_chalcedony_range = 250
			gem_quartz_range = 50
			
			if ((region == 'scorn')
			or (region == 'scorncounty')):
				mineral_salt_range = 10
			
			randnum = random.randrange(1, mineral_salt_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_salt'
				
			randnum = random.randrange(1, gem_quartz_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_quartz'
			
			randnum = random.randrange(1, gem_chalcedony_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_chalcedony'
			
			randnum = random.randrange(1, gem_agate_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_agate'
				
		elif ((Ob.ArchName == 'jungle_1')
		or (Ob.ArchName == 'jungle_2')):
			prospecttype = 'prospecthole_nothing'
			gem_gem_range =     1000000
			gem_jacinth_range =  200000
			
			randnum = random.randrange(1, mineral_gypsum_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_gypsum'
			randnum = random.randrange(1, mineral_salt_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_salt'
			randnum = random.randrange(1, mineral_gypsum_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_gypsum'
				
			randnum = random.randrange(1, zinc_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_zinc'
			
			randnum = random.randrange(1, gem_jacinth_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_jacinth'
			
			randnum = random.randrange(1, gem_gem_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_gem'
						
		elif (Ob.ArchName == 'darkforest'):
			prospecttype = 'prospecthole_nothing'
			
			mineral_pyrite_range = 20
			mineral_gypsum_range = 25
			mineral_salt_range = 30
			mineral_sulphur_range = 200
			mineral_phosphorus_range = 800
			mineral_cinnabar_range = 250
			
			randnum = random.randrange(1, mineral_dust_generic_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_dust_generic'
			randnum = random.randrange(1, mineral_graphite_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_graphite'
			randnum = random.randrange(1, mineral_pyrite_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_pyrite'
			randnum = random.randrange(1, mineral_gypsum_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_gypsum'
			randnum = random.randrange(1, mineral_salt_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_salt'
			randnum = random.randrange(1, mineral_sulphur_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_sulphur'
			randnum = random.randrange(1, mineral_phosphorus_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_phosphorus'
			randnum = random.randrange(1, mineral_cinnabar_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_cinnabar'
			
		elif ((Ob.ArchName == 'brush')
		or (Ob.ArchName == 'cyanbrush')
		or (Ob.ArchName == 'grass')
		or (Ob.ArchName == 'grass_only')
		or (Ob.ArchName == 'bosgrass')
		or (Ob.ArchName == 'bosgrass2')
		or (Ob.ArchName == 'bosgrass3')
		or (Ob.ArchName == 'bosgrass4')
		or (Ob.ArchName == 'bosgrass5')
		or (Ob.ArchName == 'bosgrass6')
		or (Ob.ArchName == 'bosgrass7')
		or (Ob.ArchName == 'bosgrass8')
		or (Ob.ArchName == 'bosgrass9')
		or (Ob.ArchName == 'grassdark')
		or (Ob.ArchName == 'grassmedium')):
 			prospecttype = 'prospecthole_nothing'
			if (region == 'gotisch'):
				coal_range = 20
			
			randnum = random.randrange(1, mineral_dust_generic_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_dust_generic'
			randnum = random.randrange(1, mineral_graphite_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_graphite'
			randnum = random.randrange(1, mineral_pyrite_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_pyrite'
			randnum = random.randrange(1, mineral_gypsum_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_gypsum'
			randnum = random.randrange(1, mineral_salt_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_salt'
			randnum = random.randrange(1, mineral_sulphur_range)
			
			randnum = random.randrange(1, coal_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_coal'
			
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_sulphur'
			randnum = random.randrange(1, mineral_phosphorus_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_phosphorus'
			randnum = random.randrange(1, mineral_cinnabar_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_cinnabar'
			
				
			randnum = random.randrange(1, gem_quartz_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_quartz'
			
			randnum = random.randrange(1, gem_chalcedony_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_chalcedony'
			
			randnum = random.randrange(1, gem_agate_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_agate'
			
			randnum = random.randrange(1, gem_tourmaline_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_tourmaline'
			randnum = random.randrange(1, gem_onyx_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_onyx'
			randnum = random.randrange(1, gem_chrysoberyl_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_chrysoberyl'
			randnum = random.randrange(1, gem_garnet_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_garnet'
			randnum = random.randrange(1, gem_aquamarine_gem_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_aquamarine_gem'
			randnum = random.randrange(1, gem_peridot_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_peridot'
			randnum = random.randrange(1, gem_topaz_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_topaz'
						
		elif ((Ob.ArchName == 'swamp')
		or (Ob.ArchName == 'marsh')
		or (Ob.ArchName == 'deep_swamp')):
			prospecttype = 'prospecthole_nothing'
			
			mineral_sulphur_range = 50
			mineral_phosphorus_range = 200
			
			randnum = random.randrange(1, mineral_dust_generic_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_dust_generic'
			randnum = random.randrange(1, mineral_graphite_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_graphite'
			randnum = random.randrange(1, mineral_pyrite_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_pyrite'
			randnum = random.randrange(1, mineral_gypsum_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_gypsum'
			randnum = random.randrange(1, mineral_salt_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_salt'
			randnum = random.randrange(1, mineral_sulphur_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_sulphur'
			randnum = random.randrange(1, mineral_phosphorus_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_phosphorus'
			randnum = random.randrange(1, mineral_cinnabar_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_cinnabar'
			
		elif ((Ob.ArchName == 'mountain')
		or (Ob.ArchName == 'mountain2')
		or (Ob.ArchName == 'mountain_2')
		or (Ob.ArchName == 'mountain4')
		or (Ob.ArchName == 'mountain5')
		or (Ob.ArchName == 's_mountain')
		or (Ob.ArchName == 'mountain3')):
			prospecttype = 'prospecthole_nothing'
			if (region == 'gotisch'):
				iron_range =    10
				coal_range =     4
		
				gem_ruby_range =    275000
				gem_emerald_range = 200000
				gem_jacinth_range = 125000
				gem_sapphire_range = 75000
				gem_amethyst_range = 25000
				gem_topaz_range =    17500
				gem_peridot_range =   8333
				gem_aquamarine_gem_range = 5000
				gem_garnet_range = 3333
				gem_chrysoberyl_range = 6000
				gem_onyx_range = 3333
				gem_tourmaline_range = 1666
				gem_agate_range = 333
				gem_chalcedony_range = 166
				gem_quartz_range = 33
			
			randnum = random.randrange(1, worthless_b_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_worthless_b'
			randnum = random.randrange(1, worthless_g_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_worthless_g'
			randnum = random.randrange(1, worthless_r_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_worthless_r'
			randnum = random.randrange(1, worthless_mixed_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_worthless_mixed'
			
			randnum = random.randrange(1, coal_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_coal'
			
				
			randnum = random.randrange(1, tin_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_tin'
			randnum = random.randrange(1, zinc_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_zinc'	
			randnum = random.randrange(1, lead_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_lead'
			randnum = random.randrange(1, iron_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_iron'
			
			randnum = random.randrange(1, gem_quartz_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_quartz'
			
			randnum = random.randrange(1, gem_chalcedony_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_chalcedony'
			
			randnum = random.randrange(1, gem_agate_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_agate'
			
			randnum = random.randrange(1, copper_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_copper'
			
			randnum = random.randrange(1, gem_tourmaline_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_tourmaline'
			randnum = random.randrange(1, gem_onyx_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_onyx'
			randnum = random.randrange(1, gem_chrysoberyl_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_chrysoberyl'
			randnum = random.randrange(1, gem_garnet_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_garnet'
				
			randnum = random.randrange(1, silver_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_silver'
			
			randnum = random.randrange(1, gem_aquamarine_gem_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_aquamarine_gem'
			randnum = random.randrange(1, gem_peridot_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_peridot'
			randnum = random.randrange(1, gem_topaz_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_topaz'
				
			randnum = random.randrange(1, gem_amethyst_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_amethyst'
				
			randnum = random.randrange(1, gold_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gold'
				
			randnum = random.randrange(1, gem_sapphire_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_sapphire'
				
			randnum = random.randrange(1, gem_jacinth_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_jacinth'
				
			randnum = random.randrange(1, gem_emerald_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_emerald'
				
			randnum = random.randrange(1, gem_ruby_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_ruby'
				
			randnum = random.randrange(1, gem_gem_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_gem'
				
			randnum = random.randrange(1, gem_mithril_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_mithril'
				
			randnum = random.randrange(1, plat_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_plat'
				
		elif ((Ob.ArchName == 'istone')
		or (Ob.ArchName == 'ipond')
		or (Ob.ArchName == 'nasty_ice')
		or (Ob.ArchName == 'evil_ice')
		or (Ob.ArchName == 'drifts')
		or (Ob.ArchName == 'cmarsh')
		or (Ob.ArchName == 'snow')
		or (Ob.ArchName == 'snow2')
		or (Ob.ArchName == 'snow3')
		or (Ob.ArchName == 'snow4')
		or (Ob.ArchName == 'snow5')
		or (Ob.ArchName == 'glacier')
		or (Ob.ArchName == 'ice')
		or (Ob.ArchName == 'ice2')):
			prospecttype = 'prospecthole_nothing'
			
			randnum = random.randrange(1, mineral_salt_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_salt'
				
			randnum = random.randrange(1, gem_chalcedony_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_chalcedony'
				
			randnum = random.randrange(1, gem_aquamarine_gem_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_aquamarine_gem'
				
			randnum = random.randrange(1, gem_sapphire_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_gem_sapphire'
		else:		
			prospecttype = 'prospecthole_nothing'
			
			randnum = random.randrange(1, mineral_dust_generic_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_dust_generic'
			randnum = random.randrange(1, mineral_graphite_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_graphite'
			randnum = random.randrange(1, mineral_pyrite_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_pyrite'
			randnum = random.randrange(1, mineral_gypsum_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_gypsum'
			randnum = random.randrange(1, mineral_salt_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_salt'
			randnum = random.randrange(1, mineral_sulphur_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_sulphur'
			randnum = random.randrange(1, mineral_phosphorus_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_phosphorus'
			randnum = random.randrange(1, mineral_cinnabar_range)
			if (randnum == 2):
				prospecttype = 'prospecthole_mineral_cinnabar'
		
	if (buildingtype == 'mine'):
		entertox = 7
		entertoy = 24
		
		if ((Ob.ArchName == 'swamp')
		or (Ob.ArchName == 'marsh')
		or (Ob.ArchName == 'deep_swamp')
		or(Ob.ArchName == 'jungle_1')
		or (Ob.ArchName == 'jungle_2')):
			housearch = 'mineentrance3'
		elif (region == 'gotisch'):
			housearch = 'mineentrance1'
		elif ((Ob.ArchName == 'small_stones')
		or (Ob.ArchName == 'steppe')
		or (Ob.ArchName == 'pstone_1')
		or (Ob.ArchName == 'pstone_2')
		or (Ob.ArchName == 'medium_stones')
		or (Ob.ArchName == 'large_stones')
		or (Ob.ArchName == 'pstone_3')
		or (Ob.ArchName == 'pstone_4')
		or (Ob.ArchName == 'pstone_5')
		or (Ob.ArchName == 'crater')
		or (Ob.ArchName == 'lostlabdesert')
		or (Ob.ArchName == 'lostlabdesert2')
		or (Ob.ArchName == 'lostlabdesert3')
		or (Ob.ArchName == 'lostlabdesert4')
		or (Ob.ArchName == 'lostlabdesert5')
		or (Ob.ArchName == 'lostlabgreysand')
		or (Ob.ArchName == 'steppelight')
		or (Ob.ArchName == 'desert')
		or (Ob.ArchName == 'dunes')
		or (region == 'navar')
		or (region == 'navarempire')
		or (region == 'celvear')
		or (region == 'lostwages')):
			housearch = 'mineentrance2'
		else:
			housearch = 'mineentrance1'
		
		
        	housemap = 'mine_standard'
	
		if (prospecttype == 'prospecthole_coal'):
			rmapminetype = 'coal'
			housename = 'coal mine'
			itemswaparchname = 'rockcoal1'
		elif (prospecttype == 'prospecthole_copper'):
			rmapminetype = 'copper'
			housename = 'copper mine'
			itemswaparchname = 'copperore1kg'
		elif (prospecttype == 'prospecthole_gold'):
			rmapminetype = 'gold'
			housename = 'gold mine'
			itemswaparchname = 'goldore1kg'
		elif (prospecttype == 'prospecthole_iron'):
			rmapminetype = 'iron'
			housename = 'iron mine'
			itemswaparchname = 'ironore1kg'
		elif (prospecttype == 'prospecthole_lead'):
			rmapminetype = 'lead'
			housename = 'lead mine'
			itemswaparchname = 'leadore1kg'
		elif (prospecttype == 'prospecthole_nothing'):
			rmapminetype = 'nothing'
			housename = 'mine'
			itemswaparchname = 'rock2'
		elif (prospecttype == 'prospecthole_plat'):
			rmapminetype = 'plat'
			housename = 'platinum mine'
			itemswaparchname = 'platore1kg'
		elif (prospecttype == 'prospecthole_silver'):
			rmapminetype = 'silver'
			housename = 'silver mine'
			itemswaparchname = 'silverore1kg'
		elif (prospecttype == 'prospecthole_tin'):
			rmapminetype = 'tin'
			housename = 'tin mine'
			itemswaparchname = 'tinore1kg'
		elif (prospecttype == 'prospecthole_worthless_b'):
			rmapminetype = 'worthless_b'
			housename = 'mine'
			itemswaparchname = 'worthlessoresmall_b'
		elif (prospecttype == 'prospecthole_worthless_g'):
			rmapminetype = 'worthless_g'
			housename = 'mine'
			itemswaparchname = 'worthlessoresmall_g'
		elif (prospecttype == 'prospecthole_worthless_mixed'):
			rmapminetype = 'worthless_mixed'
			housename = 'mine'
			itemswaparchname = 'worthlessoresmall_b'
		elif (prospecttype == 'prospecthole_worthless_r'):
			rmapminetype = 'worthless_r'
			housename = 'mine'
			itemswaparchname = 'worthlessoresmall_r'
		elif (prospecttype == 'prospecthole_worthless_y'):
			rmapminetype = 'worthless_y'
			housename = 'mine'
			itemswaparchname = 'worthlessoresmall_y'
		elif (prospecttype == 'prospecthole_zinc'):
			rmapminetype = 'zinc'
			housename = 'zinc mine'
			itemswaparchname = 'zincore1kg'
		elif (prospecttype == 'prospecthole_gem_agate'):
			rmapminetype = 'gem_agate'
			housename = 'agate mine'
			itemswaparchname = 'agate'
		elif (prospecttype == 'prospecthole_gem_amethyst'):
			rmapminetype = 'gem_amethyst'
			housename = 'amethyst mine'
			itemswaparchname = 'amethyst'
		elif (prospecttype == 'prospecthole_gem_aquamarine_gem'):
			rmapminetype = 'gem_aquamarine_gem'
			housename = 'aquamarine mine'
			itemswaparchname = 'aquamarine_gem'
		elif (prospecttype == 'prospecthole_gem_chalcedony'):
			rmapminetype = 'gem_chalcedony'
			housename = 'chalcedony mine'
			itemswaparchname = 'chalcedony'
		elif (prospecttype == 'prospecthole_gem_chrysoberyl'):
			rmapminetype = 'gem_chrysoberyl'
			housename = 'chrysoberyl mine'
			itemswaparchname = 'chrysoberyl'
		elif (prospecttype == 'prospecthole_gem_emerald'):
			rmapminetype = 'gem_emerald'
			housename = 'emerald mine'
			itemswaparchname = 'emerald'
		elif (prospecttype == 'prospecthole_gem_garnet'):
			rmapminetype = 'gem_garnet'
			housename = 'garnet mine'
			itemswaparchname = 'garnet'
		elif (prospecttype == 'prospecthole_gem_gem'):
			rmapminetype = 'gem_gem'
			housename = 'diamond mine'
			itemswaparchname = 'gem'
		elif (prospecttype == 'prospecthole_gem_jacinth'):
			rmapminetype = 'gem_jacinth'
			housename = 'jacinth mine'
			itemswaparchname = 'jacinth'
		elif (prospecttype == 'prospecthole_gem_mithril'):
			rmapminetype = 'gem_mithril'
			housename = 'mithril mine'
			itemswaparchname = 'mithril'
		elif (prospecttype == 'prospecthole_gem_onyx'):
			rmapminetype = 'gem_onyx'
			housename = 'onyx mine'
			itemswaparchname = 'onyx'
		elif (prospecttype == 'prospecthole_gem_peridot'):
			rmapminetype = 'gem_peridot'
			housename = 'peridot mine'
			itemswaparchname = 'peridot'
		elif (prospecttype == 'prospecthole_gem_quartz'):
			rmapminetype = 'gem_quartz'
			housename = 'quartz mine'
			itemswaparchname = 'quartz'
		elif (prospecttype == 'prospecthole_gem_ruby'):
			rmapminetype = 'gem_ruby'
			housename = 'ruby mine'
			itemswaparchname = 'ruby'
		elif (prospecttype == 'prospecthole_gem_sapphire'):
			rmapminetype = 'gem_sapphire'
			housename = 'sapphire mine'
			itemswaparchname = 'sapphire'
		elif (prospecttype == 'prospecthole_gem_topaz'):
			rmapminetype = 'gem_topaz'
			housename = 'topaz mine'
			itemswaparchname = 'topaz'
		elif (prospecttype == 'prospecthole_gem_tourmaline'):
			rmapminetype = 'gem_tourmaline'
			housename = 'tourmaline mine'
			itemswaparchname = 'tourmaline'
		elif (prospecttype == 'prospecthole_mineral_cinnabar'):
			rmapminetype = 'mineral_cinnabar'
			housename = 'cinnabar mine'
			itemswaparchname = 'cinnabar'
		elif (prospecttype == 'prospecthole_mineral_dust_generic'):
			rmapminetype = 'mineral_dust_generic'
			housename = 'mine'
			itemswaparchname = 'dust_generic'
		elif (prospecttype == 'prospecthole_mineral_graphite'):
			rmapminetype = 'mineral_graphite'
			housename = 'graphite mine'
			itemswaparchname = 'graphite'
		elif (prospecttype == 'prospecthole_mineral_gypsum'):
			rmapminetype = 'mineral_gypsum'
			housename = 'gypsum mine'
			itemswaparchname = 'gypsum'
		elif (prospecttype == 'prospecthole_mineral_phosphorus'):
			rmapminetype = 'mineral_phosphorus'
			housename = 'phosphorus mine'
			itemswaparchname = 'phosphorus'
		elif (prospecttype == 'prospecthole_mineral_pyrite'):
			rmapminetype = 'mineral_pyrite'
			housename = 'pyrite mine'
			itemswaparchname = 'pyrite'
		elif (prospecttype == 'prospecthole_mineral_salt'):
			rmapminetype = 'mineral_salt'
			housename = 'salt mine'
			itemswaparchname = 'salt'
		elif (prospecttype == 'prospecthole_mineral_sulphur'):
			rmapminetype = 'mineral_sulphur'
			housename = 'sulphur mine'
			itemswaparchname = 'sulphur'
		else:
			isplayer = 0
			me.Map.Print('This scroll is empty')
	elif (buildingtype == 'prospect'):
		playergetkeys = 0
		region = '%s' % (me.Map.Region.Name)
    		
		housename = 'prospect'

		if (prospecttype == 'prospecthole_coal'):
			housearch = 'prospecthole_coal'
		elif (prospecttype == 'prospecthole_copper'):
			housearch = 'prospecthole_copper'
		elif (prospecttype == 'prospecthole_gold'):
			housearch = 'prospecthole_gold'
		elif (prospecttype == 'prospecthole_iron'):
			housearch = 'prospecthole_iron'
		elif (prospecttype == 'prospecthole_lead'):
			housearch = 'prospecthole_lead'
		elif (prospecttype == 'prospecthole_nothing'):
			housearch = 'prospecthole_nothing'
		elif (prospecttype == 'prospecthole_plat'):
			housearch = 'prospecthole_plat'
		elif (prospecttype == 'prospecthole_silver'):
			housearch = 'prospecthole_silver'
		elif (prospecttype == 'prospecthole_tin'):
			housearch = 'prospecthole_tin'
		elif (prospecttype == 'prospecthole_worthless_b'):
			housearch = 'prospecthole_worthless_b'
		elif (prospecttype == 'prospecthole_worthless_g'):
			housearch = 'prospecthole_worthless_g'
		elif (prospecttype == 'prospecthole_worthless_mixed'):
			housearch = 'prospecthole_worthless_mixed'
		elif (prospecttype == 'prospecthole_worthless_r'):
			housearch = 'prospecthole_worthless_r'
		elif (prospecttype == 'prospecthole_worthless_y'):
			housearch = 'prospecthole_worthless_y'
		elif (prospecttype == 'prospecthole_zinc'):
			housearch = 'prospecthole_zinc'
		elif (prospecttype == 'prospecthole_gem_agate'):
			housearch = 'prospecthole_gem_agate'
		elif (prospecttype == 'prospecthole_gem_amethyst'):
			housearch = 'prospecthole_gem_amethyst'
		elif (prospecttype == 'prospecthole_gem_aquamarine_gem'):
			housearch = 'prospecthole_gem_aquamarine_gem'
		elif (prospecttype == 'prospecthole_gem_chalcedony'):
			housearch = 'prospecthole_gem_chalcedony'
		elif (prospecttype == 'prospecthole_gem_chrysoberyl'):
			housearch = 'prospecthole_gem_chrysoberyl'
		elif (prospecttype == 'prospecthole_gem_emerald'):
			housearch = 'prospecthole_gem_emerald'
		elif (prospecttype == 'prospecthole_gem_garnet'):
			housearch = 'prospecthole_gem_garnet'
		elif (prospecttype == 'prospecthole_gem_gem'):
			housearch = 'prospecthole_gem_gem'
		elif (prospecttype == 'prospecthole_gem_jacinth'):
			housearch = 'prospecthole_gem_jacinth'
		elif (prospecttype == 'prospecthole_gem_mithril'):
			housearch = 'prospecthole_gem_mithril'
		elif (prospecttype == 'prospecthole_gem_onyx'):
			housearch = 'prospecthole_gem_onyx'
		elif (prospecttype == 'prospecthole_gem_peridot'):
			housearch = 'prospecthole_gem_peridot'
		elif (prospecttype == 'prospecthole_gem_quartz'):
			housearch = 'prospecthole_gem_quartz'
		elif (prospecttype == 'prospecthole_gem_ruby'):
			housearch = 'prospecthole_gem_ruby'
		elif (prospecttype == 'prospecthole_gem_sapphire'):
			housearch = 'prospecthole_gem_sapphire'
		elif (prospecttype == 'prospecthole_gem_topaz'):
			housearch = 'prospecthole_gem_topaz'
		elif (prospecttype == 'prospecthole_gem_tourmaline'):
			housearch = 'prospecthole_gem_tourmaline'
		elif (prospecttype == 'prospecthole_mineral_cinnabar'):
			housearch = 'prospecthole_mineral_cinnabar'
		elif (prospecttype == 'prospecthole_mineral_dust_generic'):
			housearch = 'prospecthole_mineral_dust_generic'
		elif (prospecttype == 'prospecthole_mineral_graphite'):
			housearch = 'prospecthole_mineral_graphite'
		elif (prospecttype == 'prospecthole_mineral_gypsum'):
			housearch = 'prospecthole_mineral_gypsum'
		elif (prospecttype == 'prospecthole_mineral_phosphorus'):
			housearch = 'prospecthole_mineral_phosphorus'
		elif (prospecttype == 'prospecthole_mineral_pyrite'):
			housearch = 'prospecthole_mineral_pyrite'
		elif (prospecttype == 'prospecthole_mineral_salt'):
			housearch = 'prospecthole_mineral_salt'
		elif (prospecttype == 'prospecthole_mineral_sulphur'):
			housearch = 'prospecthole_mineral_sulphur'
		else:
			isplayer = 0
			me.Map.Print('This scroll is empty')
		
elif (buildingtype == 'harbor'):
	if (buildharbor == 1):
		playergetkeys = 0
		newmapcreated = 0
		housename = 'saving harbor'
		housearch = 'sea_harbor'
	else:
		isplayer = 0
		me.Map.Print('This scroll is empty')
elif (buildingtype == 'lamp'):
	playergetkeys = 0
        newmapcreated = 0
	
	region = '%s' % (me.Map.Region.Name)

        if ((region == 'navar')
    	or (region == 'navarempire')
    	or (region == 'celvear')
    	or (region == 'lostwages')
     	or (region == 'scorn')
    	or (region == 'scorncounty')
    	or (region == 'darcap')):
		housename = 'lamppost'
                housearch = 'bright_lamppost'
	elif ((region == 'gotisch') or (interworld == 1)):
		housename = 'Laternenpfahl'
                housearch = 'bright_lamppost_ironfirepot'
	elif ((region == 'azumauindo') or (aquiferworld == 1)):
		housename = 'ranpu'
                housearch = 'bright_lamppost_east'	
        else:
                housename = 'lamppost'
                housearch = 'bright_lamppost_firepot'
elif (buildingtype == 'guild'):
    region = '%s' % (me.Map.Region.Name)
    
    entertox = 13
    entertoy = 19
    
    if (bottomworld == 1):
        housearch = 'keep_sym_noflag_greco'
        housemap = 'smallguild_greco'
        housename = 'small guild'
    elif ((region == 'navar')
    or (region == 'navarempire')
    or (region == 'celvear')
    or (region == 'lostwages')):
        housearch = 'keep_sym_noflag_west'
        housemap = 'smallguild_navar'
        housename = 'small guild'
    elif ((region == 'scorn')
    or (region == 'scorncounty')):
        housearch = 'keep_sym_noflag'
        housemap = 'smallguild_scorn'
        housename = 'small guild'
    elif (region == 'darcap'):
        housearch = 'keep_sym_noflag_northwest'
        housemap = 'smallguild_darcap'
        housename = 'small guild'
    elif ((region == 'gotisch') or (interworld == 1)):
        housearch = 'keep_sym_noflag_goth'
        housemap = 'smallguild_gotisch'
        housename = 'Kleiner Zunft'
    elif ((region == 'azumauindo') or (aquiferworld == 1)):
        housearch = 'smallguild1_east'
        housemap = 'smallguild_azumauindo'
        housename = 'chiisai shougyoukumiai'
    elif ((region == 'brest')
    or (region == 'brittany')):
        housearch = 'keep_sym_noflag_fant'
        housemap = 'smallguild_brest'
        housename = 'small guild'
    elif ((region == 'antarctica') or (antarcticworld == 1)):
        housearch = 'keep_sym_noflag_misc'
        housemap = 'smallguild_darcap'
        housename = 'small guild'
    elif (region == 'wilderness'):
        housearch = 'keep_sym_noflag_misc'
        housemap = 'smallguild_standard'
        housename = 'small guild'
    else:
        housearch = 'keep_sym_noflag_misc'
        housemap = 'smallguild_standard'
        housename = 'small guild'
    
else:
    isplayer = 0
    me.Map.Print('This scroll is empty')

    
if ((isplayer == 1) and (newmapcreated == 1)):
        
    template = templatedir+housemap
    
    Ob = me
    Ob = Ob.Below
    floorarchname = Ob.ArchName
    
    building = Crossfire.CreateObjectByName(housearch)
    building.InsertInto(me)
    me.Drop(building)
    building.Name = "%s's %s" % (me.Name, housename)
    
    if (unique == 1):
	building.Unique = True
    
    snamebuildtype = '%s_%s' % (sname, buildingtype)
    newfile = '%s%s_%s_%s_%s' % (builtdir, smap, sx, sy, snamebuildtype)
    
    eapply = Crossfire.CreateObjectByName('event_apply')
    eapply.InsertInto(building)
    eapply.Name = '/world_built%s_%s_%s_%s' % ( smap, sx, sy, snamebuildtype)
    eapply.Title = 'Python'
    eapply.Slaying = '/python/misc/buildteleport.py'
    building.HP = entertox
    building.SP = entertoy
    
    shutil.copyfile(template, newfile)
    if (buildingtype == 'tob'):
	templateup = '%s_up1' % (template)
	templatedown = '%s_down1' % (template)
	tupmap = '/world_built%s_%s_%s_%s_up1' % ( smap, sx, sy, snamebuildtype)
	tdownmap = '/world_built%s_%s_%s_%s_down1' % ( smap, sx, sy, snamebuildtype)
	tmainmap = '/world_built%s_%s_%s_%s' % ( smap, sx, sy, snamebuildtype)
	newfileup = '%s_up1' % (newfile)
	newfiledown = '%s_down1' % (newfile)
	shutil.copyfile(templateup, newfileup)
	shutil.copyfile(templatedown, newfiledown)
    
    file = open(newfile, "r")
    string = file.read()
    file.close()
    string = string.replace("region wilderness", "region "+region)
    string = string.replace("arch neverusethisarch", "arch "+floorarchname)
    string = string.replace("arch dontusethisarch", "arch "+itemswaparchname)
    string = string.replace("slaying REPLACE_ME", "slaying /"+smap)
    string = string.replace("slaying PLAYER_FORCE", "slaying key_"+newfile)
    string = string.replace("hp 9999", "hp "+sx)
    string = string.replace("sp 9999", "sp "+sy)
    if (buildingtype == 'tob'):
	string = string.replace("slaying UP_1", "slaying "+tupmap)
	string = string.replace("slaying DOWN_1", "slaying "+tdownmap)
	string = string.replace("slaying MAIN_FLOOR", "slaying "+tmainmap)
    elif (buildingtype == 'farm'):
    	string = string.replace("nrof 10", "nrof "+itemswapnumber)
    elif (buildingtype == 'mine'):
    	string = string.replace("MINETYPE", rmapminetype)    
    file = open(newfile, "w")
    file.write(string)
    file.close()
    
    if (buildingtype == 'tob'):
	#Prepare and write upper floor 1 of tob tower:
	file = open(newfileup, "r")
	string = file.read()
	file.close()
	string = string.replace("region wilderness", "region "+region)
	string = string.replace("slaying REPLACE_ME", "slaying /"+smap)
	string = string.replace("slaying PLAYER_FORCE", "slaying key_"+newfile)
	string = string.replace("hp 9999", "hp "+sx)
	string = string.replace("sp 9999", "sp "+sy)
	string = string.replace("slaying UP_1", "slaying "+tupmap)
	string = string.replace("slaying DOWN_1", "slaying "+tdownmap)
	string = string.replace("slaying MAIN_FLOOR", "slaying "+tmainmap)
	file = open(newfileup, "w")
	file.write(string)
	file.close()
	#Prepare and write basement of tob tower:
	file = open(newfiledown, "r")
	string = file.read()
	file.close()
	string = string.replace("region wilderness", "region "+region)
	string = string.replace("slaying REPLACE_ME", "slaying /"+smap)
	string = string.replace("slaying PLAYER_FORCE", "slaying key_"+newfile)
	string = string.replace("hp 9999", "hp "+sx)
	string = string.replace("sp 9999", "sp "+sy)
	string = string.replace("slaying UP_1", "slaying "+tupmap)
	string = string.replace("slaying DOWN_1", "slaying "+tdownmap)
	string = string.replace("slaying MAIN_FLOOR", "slaying "+tmainmap)
	file = open(newfiledown, "w")
	file.write(string)
	file.close()
   	
    
    force = Crossfire.CreateObjectByName("force")
    force.Slaying = "key_"+newfile
    force.Speed = 0
    force.InsertInto(me)
    
    if (playergetkeys == 1):
    	key = Crossfire.CreateObjectByName("key2")
    	key.Slaying = "key_"+newfile
    	key.Name = "%s's %s key" % (me.Name, housename)
    	key.NamePl = "%s's %s keys" % (me.Name, housename)
    	key.Title = 'of player built building'
    	key.Quantity = 5
    	key.InsertInto(me)
    
    me.Map.Print('You have sucessfully constructed your building.')
    
    item = Crossfire.WhoAmI()
    if item:
	if (item.Quantity >= 2):
	    item.Quantity = item.Quantity - 1
	else:
	    item.Remove()
elif ((isplayer == 1) and (newmapcreated == 0)):
    building = Crossfire.CreateObjectByName(housearch)
    building.InsertInto(me)
    me.Drop(building)
    building.Name = "%s's %s" % (me.Name, housename)
    
    if (unique == 1):
	building.Unique = True
	
    me.Map.Print('You have sucessfully constructed your building.')
    
    item = Crossfire.WhoAmI()
    if item:
	if (item.Quantity >= 2):
	    item.Quantity = item.Quantity - 1
	else:
	    item.Remove()  
    

#If not player, do nothing
