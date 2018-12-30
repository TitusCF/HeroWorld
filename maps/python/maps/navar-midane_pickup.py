import Crossfire
import random

obj = Crossfire.WhoAmI()

Crossfire.SetReturnValue(1)

messages = ["Midane says: %p, you thief! Leave this %i where it is!",
	"Midane says: Tell me, %p, what do you plan to do with this %i?",
	"Midane says: I'm sure you don't need any %i, do you, %p?"
	]

msg = messages[random.randint(0, len(messages)-1)]
msg = msg.replace('%i', obj.Name)
msg = msg.replace('%p', Crossfire.WhoIsActivator().Name)

obj.Map.Print(msg, Crossfire.MessageFlag.NDI_NAVY + Crossfire.MessageFlag.NDI_UNIQUE)
