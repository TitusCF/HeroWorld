import Crossfire
import random
#This script allows the orange worm to spit fire etc

worm = Crossfire.WhoAmI()
me = Crossfire.WhoIsOther()

me.Map.Print("The %s spits up fire." % (worm.Name)) 

spellob = Crossfire.CreateObjectByName("spell_burning_hands")
worm.CastAbility(spellob, 0, "")
spellob.Remove()

item = Crossfire.CreateObjectByName("coldmagma")
item.InsertInto(me)
me.Drop(item)


