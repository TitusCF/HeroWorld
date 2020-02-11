import Crossfire
spellob = Crossfire.CreateObjectByName("spell_cause_red_death")
Crossfire.WhoAmI().CastAbility(spellob, 0, "")
spellob.Remove()



