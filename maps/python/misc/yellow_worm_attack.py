import Crossfire
import random
#This script allows the yellow worm to spit acid etc

worm = Crossfire.WhoAmI()
me = Crossfire.WhoIsOther()

me.Map.Print("The %s spits up acid and vitriol." % (worm.Name)) 

spellob = Crossfire.CreateObjectByName("spell_vitriol_splash")
worm.CastAbility(spellob, 0, "")
spellob.Remove()

item = Crossfire.CreateObjectByName("acid_pool")
item.InsertInto(worm)
worm.Drop(item)


