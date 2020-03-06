import Crossfire
spellob = Crossfire.CreateObjectByName("spell_vitriol_splash")
Crossfire.WhoAmI().CastAbility(spellob, 0, "")
spellob.Remove()



