import Crossfire
spellob = Crossfire.CreateObjectByName("spell_mystic_fist")
Crossfire.WhoAmI().CastAbility(spellob, 0, "")
spellob.Remove()



