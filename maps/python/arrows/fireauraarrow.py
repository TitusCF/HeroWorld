import Crossfire
#Rednaxela helped me alot with this, gave me all the code --MikeeUSA--
#Fire burning hands in all directions (splashback).
spellob = Crossfire.CreateObjectByName("spell_vitriol_splash")
Crossfire.WhoAmI().CastAbility(spellob, 0, "")
spellob.Remove()
#Cast create bomb spell (Igniting the bomb at the end of the arrow)
#Thank you Rednaxela.

