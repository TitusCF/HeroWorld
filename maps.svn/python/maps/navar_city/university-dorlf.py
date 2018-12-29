import Crossfire

message = Crossfire.WhatIsMessage()

if message == 'lorkas' or message == 'Lorkas':
    who = Crossfire.WhoAmI()
    who.Event(who, who, "Lorkas", 0)
