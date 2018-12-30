'''
This script is part of the Witherspoon quest, that starts in /scorn/mansion/witherspoon_manor_attic.
Check the README file in the same directory as this file for more details.

Script for Rolanda, in /scorn/houses/rolanda.

Support to be called for TIME, TIMER and CUSTOM events.
'''

import Crossfire
import CFMove
import random

whoami = Crossfire.WhoAmI()
whoisother = Crossfire.WhoIsOther()
command = Crossfire.WhatIsMessage()
event = Crossfire.WhatIsEvent()
pl = Crossfire.WhoIsActivator()


# Message color for player
color = Crossfire.MessageFlag.NDI_BROWN

# coordinates to look for chairs
search_chairs = [ [ 0, -1 ], [ 0, 1 ], [ -1, 0 ], [ 1, 0 ] ]

def do_give():
	'''handle the player giving an item.'''
	if whoisother.ReadKey('special_item') != 'ghost_dagger':
		whoami.Say('And what am I supposed to do with this %s.'%whoisother.Name)
		return

	whoami.Say('Ohhhhhhhh... This, this dagger...');
	pl.Message('%s puts her hand to her forehead, and staggers.'%whoami.Name)
	whoami.WriteKey('witherspoon_saw_dagger', '1', 1)
	whoami.WriteKey('witherspoon_seated', '', 1)
	whoami.WriteKey('explaining_for', pl.Name, 1)

def search_chair():
	'''if fainting, try to find a chair when TIME happens.'''
	for search in range(0, len(search_chairs)):
		x = whoami.X + search_chairs[search][0]
		y = whoami.Y + search_chairs[search][1]
		obj = whoami.Map.ObjectAt(x, y)
		while obj:
			if obj.Name == 'chair':
				whoami.WriteKey('witherspoon_seated', '1', 1)
				whoami.Say('Thank you very much.')
				whoami.WriteKey('chair_x', str(x), 1)
				whoami.WriteKey('chair_y', str(y), 1)
				return
			obj = obj.Above

def move_to_chair():
	'''trying to move to first found chair.'''
	m = CFMove.get_object_to(whoami, int(whoami.ReadKey('chair_x')), int(whoami.ReadKey('chair_y')))
	if m == 0:
		whoami.WriteKey('witherspoon_seated', '2', 1)
		whoami.Map.Print('%s sits on the chair.'%whoami.Name, color)
		whoami.Say('I shall explain everything...')
		whoami.Map.Print('%s starts sobbing.'%whoami.Name, color)
		whoami.CreateTimer(random.randint(5, 10), 1)
	elif m == 2:
		whoami.Say('Please let me sit...')

def explain():
	'''explanation of the ghost story. Let Rolanda be still from now on.'''

	pl = Crossfire.FindPlayer(whoami.ReadKey('explaining_for'))
	if pl == None:
		# just in case someone else is around ^_-
		whoami.Say('Tss, people just aren\'t patient...')
		return
	pl.WriteKey('witherspoon_know_all', '1', 1)

	whoami.Say('See, this dagger...')
	whoami.Say('Alfred wanted...')
	whoami.Map.Print('%s sighes deeply, and goes on.'%whoami.Name)
	whoami.Say('he wanted to have some, shall I say, different experience, the kind a man and a woman can have together.')
	whoami.Say('And since he likes pain, he asked to suffer, to see if the pain would... excite him.')
	whoami.Map.Print('%s sobs'%whoami.Name)
	whoami.Say('But he didn\'t think he would die!')
	whoami.Say('So I... I hide the body, hoping to forget him.')
	whoami.Say('But now I can\'t forget him! Ever!')
	pl.Write('You wonder what the ghost (Alfred, according to %s) will make of that...'%whoami.Name, color)

if event.Subtype == Crossfire.EventType.USER and command == 'give':
	do_give()
elif event.Subtype == Crossfire.EventType.TIME:
	if whoami.ReadKey('witherspoon_saw_dagger') != '':
		# No moving while fainting
		Crossfire.SetReturnValue(1)
		if whoami.ReadKey('witherspoon_seated') == '':
			search_chair()
		elif whoami.ReadKey('witherspoon_seated') == '1':
			move_to_chair()
elif event.Subtype == Crossfire.EventType.SAY:
	if whoami.ReadKey('witherspoon_saw_dagger') != '':
		# No talking while fainting
		Crossfire.SetReturnValue(1)
		whoami.Say('Ohhhhhhh......')
		pl.Message('%s seems to be ready to faint.'%whoami.Name)
elif event.Subtype == Crossfire.EventType.TIMER:
	if whoami.ReadKey('witherspoon_seated') == '2':
		explain()
