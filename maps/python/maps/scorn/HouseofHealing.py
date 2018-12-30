import Crossfire
from CFDataFile import CFDataFile, CFData

who = Crossfire.WhoAmI()
event = Crossfire.WhatIsEvent()
player = Crossfire.WhoIsActivator()
message = Crossfire.WhatIsMessage().lower()

# questions giving free restoration
questions = [
	'please tell me the name of the town we are in.',
	'tell me the name of the tavern east of Scorn.'
]
# answers to above questions
answers = ['scorn', 'goth']
# level to remove depletion
levels = [ 5, 5 ]

def player_status():
	header = ['uses']
	data = CFData('Scorn_HouseOfHealing', header)
	if data.exist(player.Name):
		return int(data.get_record(player.Name)['uses'])
	return 0

def player_set_status(uses):
	header = ['uses']
	data = CFData('Scorn_HouseOfHealing', header)
	record = { '#' : player.Name, 'uses' : uses }
	data.put_record(record)

def greet():
	who.Say('Welcome to the house of healing!\nThis is the place where injured and ill people get cured of their torments.')

def do_say():
	uses = player_status()
	
	if (uses < len(questions)):
		if (message == answers[uses]):
			who.Say('Correct! Be restored!');
			result = player.RemoveDepletion(5)
			player_set_status(uses + 1)
			return
		greet()
		who.Say('If you wish me to restore your stats, %s'%questions[uses])
		return
	
	greet()

if (event.Subtype == Crossfire.EventType.SAY):
	do_say()
	

Crossfire.SetReturnValue(1)
