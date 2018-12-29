import Crossfire
import random

obj = Crossfire.WhoAmI()

Crossfire.SetReturnValue(1)

messages = ["Midane says: I don't see what could interest you in this %s.",
	"Midane says: Trust me, there is nothing interesting in this %s.",
	"Midane says: Leave this poor %s alone!"
	]

obj.Map.Print(messages[random.randint(0, len(messages)-1)] %obj.Name, Crossfire.MessageFlag.NDI_NAVY + Crossfire.MessageFlag.NDI_UNIQUE)
