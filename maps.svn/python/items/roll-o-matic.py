# Script for the roll-o-matic.
# Idea courtesy Alex Schultz.
#
# Copyright 2007 Nicolas Weeger
# Released as GPL
#
# This script makes its item move following tiles, in direction specified by a player

import Crossfire

key_direction = 'rom_dir'
key_follow = 'rom_follow'

dir_x = [  0, 0, 1, 1, 1, 0, -1, -1, -1 ]
dir_y = [ 0, -1, -1, 0, 1, 1, 1, 0, -1 ]

def find_floor(x, y):
	obj = me.Map.ObjectAt(x, y)
	while obj != None:
		if obj.Floor == 1:
			return obj.ArchName
		obj = obj.Above
	return ''

def find_player():
	obj = me.Map.ObjectAt(me.X, me.Y)
	while obj != None:
		if obj.Type == Crossfire.Type.PLAYER:
			return obj
		obj = obj.Above
	return None

def has_floor(x, y, name):
	obj = me.Map.ObjectAt(x, y)
	while obj != None:
		if obj.Floor == 1 and obj.ArchName == name:
			return True
		obj = obj.Above
	return False


def abs_dir(d):
	while d < 1:
		d = d + 8
	while d > 8:
		d = d - 8
	return d

def handle_move():
	want_dir = me.ReadKey(key_direction)
	floor = me.ReadKey(key_follow)
	if want_dir == '' or floor == '':
		return
#	me.Map.Print('roll')
	pl = find_player()
	want_dir = int(want_dir)
	done = False
	x = me.X
	y = me.Y
	for check in [0, 1, -1]:
		d = abs_dir(want_dir + check)
		if has_floor(x + dir_x[d], y + dir_y[d], floor):
			if me.Move(d) == 0:
				continue

			if pl != None:
				pl.Move(d)
			done = True
			break

	if not done:
		me.WriteKey(key_direction, '', 1)
		me.WriteKey(key_follow, '', 1)
	return

def handle_say():
	msg = Crossfire.WhatIsMessage()
	if msg == 'stop':
		if me.ReadKey(key_direction) != '':
			me.WriteKey(key_direction, '', 1)
			me.WriteKey(key_follow, '', 1)
			me.Map.Print('The %s stops moving.'%me.Name)
		return

	want_dir = -1

	for d in Crossfire.DirectionName.keys():
		if msg == Crossfire.DirectionName[d].lower():
			want_dir = d
			break

	if want_dir == -1:
		return

	floor = find_floor(me.X, me.Y)
	if floor == '':
		return

	if me.ReadKey(key_direction) == '':
		me.Map.Print('The %s starts moving!'%me.Name)

	me.WriteKey(key_direction, str(want_dir), 1)
	me.WriteKey(key_follow, floor, 1)

def do_handle():
	if me.Map == None:
		return

	if evt.Subtype == Crossfire.EventType.SAY:
		handle_say()
	elif evt.Subtype == Crossfire.EventType.TIME:
		handle_move()


evt = Crossfire.WhatIsEvent()
me = Crossfire.WhoAmI()
pl = Crossfire.WhoIsActivator()

Crossfire.SetReturnValue(1)

do_handle()
