# Script for Midane's house.
#
# Copyright 2007 Nicolas Weeger
# Released as GPL

import Crossfire
import CFDataFile

who = Crossfire.WhoAmI()

data = CFDataFile.CFData('midane', ['state'])

if not 'on_map' in Crossfire.GetPrivateDictionary():
	on_map = []
	Crossfire.GetPrivateDictionary()['on_map'] = on_map
else:
	on_map = Crossfire.GetPrivateDictionary()['on_map']

def check_player(player):

	if player in on_map:
		return

	on_map.append(player)

	if not data.exist(player.Name):
		who.Say('Well, welcome, stranger.')
		record = { '#' : player.Name,
		'state' : 'seen' }
		data.put_record(record)
		return

	record = data.get_record(player.Name)
	if record['state'] == 'seen':
		who.Say('Welcome back, stranger.')
		return

	who.Say('Welcome back, %s'%player.Name)

def on_time():
	for player in Crossfire.GetPlayers():
		if player.Map == who.Map:
			check_player(player)
		elif on_map.count(player) != 0:
			on_map.remove(player)

on_time()
