'''
This script is part of the Witherspoon quest, that starts in /scorn/mansion/witherspoon_manor_attic.
Check the README file in the same directory as this file for more details.

Script for the ghost in Witherspoon manor (south-east of Scorn).
'''

import random
import Crossfire
from CFDialog import DialogRule, Dialog

key_angry = 'ghost_angry'
key_disappear = 'ghost_disappear'

def warn_player(player, ghost):
	'''Ghost is about to become angry, warn the player so she can flee before it's too late!'''
	if player.Level < ghost.Level:
		player.Write('You feel a very powerful force gather around the %s! Your instinct tell you you\'d rather feel fast!'%ghost.Name)
	else:
		player.Write('You feel a really powerful force gather around the %s! Time to start fleeing, maybe?'%ghost.Name)

	drop = ghost.Map.CreateObject('reeking_urine', player.X, player.Y)
	player.Write('You feel so frightened you can\'t control your bladder!')

def check_body(player, rule):
	'''Did the player already dig?'''
	if player.ReadKey('witherspoon_tomb') != 'dig':
		return True

	return False

def start_disappear(ghost, player):
	'''ghost should disappear, called for 2 endings.'''
	ghost.WriteKey(key_disappear, '1', 1)
	ghost.CreateTimer(5, 1)
	ghost.StandStill = True
	player.Write('The %s starts fading...'%ghost.Name)

def found_body(player, rule):
	'''Does the player have the body?'''
	if player.ReadKey('witherspoon_tomb') != 'dig':
		return False

	player.WriteKey('witherspoon_tomb', '', 1)
	#reset our dialog anyway - the player did dig, so quest ends (nicely or badly)
	player.WriteKey('dialog_witherspoon_ghost', '', 1)

	#try to find the body, if not found then get angry
	body = player.CheckInventory('tortured body')

	ghost = Crossfire.WhoAmI()

	if body:
		#all fine!
		body.Remove()
		start_disappear(ghost, player)
		return 1

	#oh oh, bad, ghost is getting angry!
	ghost.WriteKey(key_angry, '1', 1)
	ghost.CreateTimer(10, 1)
	ghost.Say('You fool! You lost my body!\nPrepare to feel my wrath!')
	warn_player(player, ghost)
	return False

	return has_body(player)

def can_talk(player, rule):
	'''Is the ghost angry or disappearing?
	Used for the final catch all rule so the ghost doesn't talk when angry or disappearing.
	'''
	return Crossfire.WhoAmI().ReadKey(key_angry) == '' and Crossfire.WhoAmI().ReadKey(key_disappear) == ''

def do_dialog():
	'''Main dialog routine.'''
	if not can_talk(None, None):
		return

	whoami = Crossfire.WhoAmI()
	pl = Crossfire.WhoIsActivator()
	if pl.ReadKey('witherspoon_know_all') == '1':
		# player found the real story, through Rolanda.
		whoami.Say('Oh...')
		whoami.Map.Print('The %s manages to blush.'%whoami.Name)
		whoami.Say('Now I remember. I\'m so stupid. Poor Rolanda, will she ever forgive me...')
		whoami.Say('Well, I guess I can go now. Many thanks for your help!')
		whoami.WriteKey('bonus', '1', 1)
		start_disappear(whoami, pl)
		return

	if pl.ReadKey('witherspoon_know_dagger') == '1':
		# player talked to the priest of Devourers, and knows the dagger is special.
		whoami.Say('Oh, please find who could be as cruel as to use such a horrible spell on me!')
		return

	# default dialog, just talk

	# If you ever change this key, change the value in tomb.py too!
	speech = Dialog(Crossfire.WhoIsActivator(), Crossfire.WhoAmI(), "witherspoon_ghost")

	prer = [["token","witherspoon_ghost","0"]]
	postr = [["settoken","witherspoon_ghost", "explain"]]
	rmsg = ["I was killed by surprise, and ever since I'm stuck here.\n\n"
	"If I could see my body, I could really understand I'm dead and I could rest in peace.\n\n"
	"Could you find my body, please?"
	]
	speech.addRule(DialogRule(["help","yes","how"], prer, rmsg, postr),0)

	prer = [["token","witherspoon_ghost","explain"]]
	postr = [["settoken","witherspoon_ghost", "wait"]]
	rmsg = ["I was walking near a lake west of Scorn, so maybe my body is buried here."]
	speech.addRule(DialogRule(["where","location"], prer, rmsg, postr),1)

	prer = [["token","witherspoon_ghost","explain"]]
	postr = [["settoken","witherspoon_ghost", "*"]]
	rmsg = ["Please, go find my body...", "Please, I need my body to rest in peace..."]
	speech.addRule(DialogRule(["*"], prer, rmsg, postr),2)

	prer = [["token","witherspoon_ghost","wait"]]
	postr = [["settoken","witherspoon_ghost", "*"]]
	rmsg = ["Please, go find my body.\n\nIt should be near the lake west of Scorn...", "Did you find my body yet? No?\n\nThen please, go search for it, west of Scorn there is a lake..."]
	rule = DialogRule(["*"], prer, rmsg, postr)
	rule.setPreFunction(check_body)
	speech.addRule(rule,3)

	prer = [["token","witherspoon_ghost","wait"]]
	postr = [["settoken","witherspoon_ghost", "0"]]
	rmsg = ["Thanks, you found my body!"]
	rule = DialogRule(["*"], prer, rmsg, postr)
	rule.setPreFunction(found_body)
	speech.addRule(rule,4)

	prer = [["token","witherspoon_ghost","*"]]
	postr = [["settoken","witherspoon_ghost", "*"]]
	rmsg = ["Please help me....", "Heeeeeeeelp...", "Pleaseeeee..."]
	rule = DialogRule(["*"], prer, rmsg, postr)
	rule.setPreFunction(can_talk)
	speech.addRule(rule,5)

	speech.speak(Crossfire.WhatIsMessage())

def do_angry(ghost):
	# Ghost is angry point, let's rock
	ghost.Say('Feel my wrath!')
	ghost.WriteKey(key_angry, '', 1)
	ghost.Unaggressive = 0
	ghost.NoDamage = 0
	ghost.RandomMovement = 0
	ghost.Speed = ghost.Speed * 2

def do_timer():
	'''Got a timer, what for?'''
	ghost = Crossfire.WhoAmI()
	if ghost.ReadKey(key_angry) != '':
		do_angry(ghost)
	elif ghost.ReadKey(key_disappear) != '':
		do_disappear()
	#shouldn't get there...

def do_disappear():
	'''Ghost is happy, all is fine.'''
	ghost = Crossfire.WhoAmI()
	if ghost.ReadKey(key_disappear) != '1':
		'''Hu? Not supposed to come here in this case...'''
		return

	ghost.Say('Thanks a lot! Please take those small presents as a token of my gratitude.')

	bonus = 0
	if ghost.ReadKey('bonus') == '1':
		bonus = 25

	presents = ['gem', 'ruby', 'emerald', 'pearl', 'sapphire']
	got = ghost.Map.CreateObject(presents[random.randint(0, len(presents) - 1)], ghost.X, ghost.Y)
	got.Quantity = random.randint(3 + bonus, 7 + bonus)

	ghost.Remove()

if Crossfire.WhatIsEvent().Subtype == Crossfire.EventType.SAY:
	Crossfire.SetReturnValue(1)
	do_dialog()
elif Crossfire.WhatIsEvent().Subtype == Crossfire.EventType.TIMER:
	do_timer()
