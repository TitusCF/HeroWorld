# Script for the Combat Chicken for Sentrio's farmhouse (/lake_country/sentrio_farmhouse).
#
# Copyright 2007 Nicolas Weeger
# Released as GPL
#
# This script is supposed to be called for the time event.

import Crossfire
import random
import CFMove

key_target = 'chicken_target'		# where the chicken is trying to go
key_food = 'chicken_food'		# currently eaten food
key_attacked = 'chicken_attacked'	# if set, chicken has normal monster behaviour - so it reacts when attacked
stay_on_floor = 'small_stones'		# what ground it'll stay on
# what the chicken will eat, and food increase
eat = { 'orc\'s livers' : 5, 'orc\'s hearts' : 6, 'goblin\'s livers' : 1, 'goblin\'s hearts' : 2 }

# returns floor with specific name
def has_floor(x, y, name):
	obj = Crossfire.WhoAmI().Map.ObjectAt(x, y)
	while obj != None:
		if obj.Floor == 1 and obj.ArchName == name:
			return True
		obj = obj.Above
	return False

# returns some eligible food on specified spot
def find_food(chicken, x, y):
	obj = chicken.Map.ObjectAt(x, y)
	while obj != None:
		#Crossfire.Log(Crossfire.LogMonster, obj.Name)
		if obj.NamePl in eat:
			return obj
		obj = obj.Above
	return None

# main chicken handler
def move_chicken():
	chicken = Crossfire.WhoAmI()
	if chicken.Enemy != None:
		# chicken won't let itself get killed easily!
		chicken.WriteKey(key_attacked, '1', 1)

	if chicken.ReadKey(key_attacked) != '':
		return

	Crossfire.SetReturnValue(1)
	if chicken.Map.Darkness >= 3:
		# too dark, night is for sleeping
		return

	target = chicken.ReadKey(key_target)
	if target != '':
		x = int(target.split('|')[0])
		y = int(target.split('|')[1])
		if CFMove.get_object_to(chicken, x, y) != 0:
			return
		# target found, let's try to eat it
		food = find_food(chicken, x, y)
		chicken.WriteKey(key_target, '', 1)
		if food != None:
			chicken.Map.Print('The %s eats the %s!'%(chicken.Name, food.Name))
			got = chicken.ReadKey(key_food)
			if got == '':
				got = 0
			else:
				got = int(got)
			got = got + eat[food.NamePl]
			# drop an egg?
			if random.randint(1, 100) <= ( got * 2 ):
				egg = chicken.Map.CreateObject('chicken_egg', chicken.X, chicken.Y)
				egg.Name = 'Combat Chicken egg'
				egg.NamePl = 'Combat Chicken eggs'
				egg.Quantity = 1
				chicken.Map.Print('The %s lays an egg!'%chicken.Name)
				got = 0
			chicken.WriteKey(key_food, str(got), 1)
			food.Quantity = food.Quantity - 1
			return
	else:
		# try to find some food
		#chicken.Map.Print('find food...')
		food = None
		for x in range(-3, 4):
			for y in range(-3, 4):
				food = find_food(chicken, chicken.X + x, chicken.Y + y)
				#chicken.Map.Print('find food %d %d...'%(chicken.X + x, chicken.Y + y))
				if food != None:
					target = '%d|%d'%(food.X, food.Y)
					chicken.WriteKey(key_target, target, 1)
					#chicken.Map.Print('got food %s'%target)
					break
			if food != None:
				break

	# nothing found, random walk
	for test in [1, 10]:
		dir = random.randint(1, 8)
		if (has_floor(chicken.X + CFMove.dir_x[dir], chicken.Y + CFMove.dir_y[dir], stay_on_floor)):
			chicken.Move(dir)
			Crossfire.SetReturnValue(1)
			return


move_chicken()
