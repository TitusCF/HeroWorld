import Crossfire
#Rednaxela helped me alot with this, gave me all the code --MikeeUSA--
#Fire burning hands in all directions (splashback).
spellob = Crossfire.CreateObjectByName("spell_burning_hands")
Crossfire.WhoAmI().CastAbility(spellob, 0, "")
spellob.Remove()
#Fire burning hands "forward" (whatever direction the arrow is going in).
#spellob = Crossfire.CreateObjectByName("spell_burning_hands")
#Crossfire.WhoAmI().CastAbility(spellob, Crossfire.WhoAmI().Direction, "")
#spellob.Remove()
#Cast create bomb spell (Igniting the bomb at the end of the arrow)
spellob = Crossfire.CreateObjectByName("spell_create_bomb")
Crossfire.WhoAmI().CastAbility(spellob, 0, "")
spellob.Remove()
#Thank you Rednaxela.

