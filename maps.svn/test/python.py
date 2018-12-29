import Crossfire
import random
#import CFLog

Crossfire.SetReturnValue( 1 )

whoami=Crossfire.WhoAmI()

def do_help():
	whoami.Say('Usage: say <test name>\nAvailable tests:')
	whoami.Say(' - arch: archetypes-related tests')
	whoami.Say(' - maps: maps-related tests')
	whoami.Say(' - party: party-related tests')
	whoami.Say(' - region: party-related tests')
	whoami.Say(' - ref: some checks on objects references')
	whoami.Say(' - mark: marked item')
	whoami.Say(' - memory: storage-related tests')

def do_arch():
	archs = Crossfire.GetArchetypes()
	whoami.Say('%d archetypes'%len(archs))
	which = random.randint(0,len(archs))
	arch = archs[which]
	whoami.Say('random = %s'%arch.Name)

	arch = Crossfire.WhoIsActivator().Archetype()
	whoami.Say('your archetype is %s'%arch.Name)

def do_maps():
	maps = Crossfire.GetMaps()
	whoami.Say('%d maps loaded'%len(maps))
	for map in maps:
		whoami.Say('%s   -> %d players'%(map.Name,map.Players))
#activator=Crossfire.WhoIsActivator()

def do_party():
	parties = Crossfire.GetParties()
	whoami.Say('%d parties'%len(parties))
	for party in parties:
		whoami.Say('%s'%(party.Name))
		players = party.GetPlayers()
		for player in players:
			whoami.Say('   %s'%player.Name)
	if len(parties) >= 2:
		Crossfire.WhoIsActivator().Party = parties[1]
		whoami.Say('changed your party!')

def do_region():
	whoami.Say('Known regions, region for current map is signaled by ***')
	cur = whoami.Map.Region
	whoami.Say('This map\'s region is %s'%(cur.Name))
	regions = Crossfire.GetRegions()
	whoami.Say('%d regions'%len(regions))
	for region in regions:
		if cur == region:
			whoami.Say('*** %s - %s'%(region.Name,region.Longname))
		else:
			whoami.Say('%s - %s'%(region.Name,region.Longname))
	parent = cur.GetParent()
	if parent:
		whoami.Say('Parent is %s'%parent.Name)
	else:
		whoami.Say('Region without parent')

def do_activator():
	who = Crossfire.WhoIsActivator()
	who2 = Crossfire.WhoIsOther()
	who3 = Crossfire.WhoAmI()
	who = 0
	who2 = 0
	who3 = 0
	whoami.Say('let\'s hope no reference crash!')

def do_marker():
	who = Crossfire.WhoIsActivator()
	obj = who.MarkedItem
	if obj:
		whoami.Say(' your marked item is: %s'%obj.Name)
		mark = obj.Below
	else:
		whoami.Say(' no marked item')
		mark = who.Inventory
	while (mark) and (mark.Invisible):
		mark = mark.Below
	who.MarkedItem = mark
	whoami.Say('Changed marked item!')

def do_memory():
	whoami.Say('Value save test')
	dict = Crossfire.GetPrivateDictionary()
	if dict.has_key('s'):
		x = dict['s']
		whoami.Say(' x was %d'%x)
		x = x + 1
	else:
		x = 0
		whoami.Say(' new x')

	dict['s'] = x
		
whoami.Say( 'plugin test' )

topic = Crossfire.WhatIsMessage().split()
#whoami.Say('topic = %s'%topic)
#whoami.Say('topic[0] = %s'%topic[0])
if topic[0] == 'arch':
	do_arch()
elif topic[0] == 'maps':
	do_maps()
elif topic[0] == 'party':
	do_party()
elif topic[0] == 'region':
	do_region()
elif topic[0] == 'mark':
	do_marker()
elif topic[0] == 'ref':
	do_activator()
elif topic[0] == 'memory':
	do_memory()
else:
	do_help()
