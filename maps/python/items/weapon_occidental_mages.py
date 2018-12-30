import Crossfire
import random

me = Crossfire.WhoAmI()
ac = Crossfire.WhoIsActivator()
r  = random.random()

if (r <= 0.01):
	if me.Weight == me.Archetype.Clone.Weight:
		ac.Write("Your weapon suddenly seems lighter!")
		me.Weight = 9 * me.Archetype.Clone.Weight / 10
		me.Dam = me.Archetype.Clone.Dam
		me.LastSP = me.Archetype.Clone.LastSP
elif (r <= 0.02):
	if me.Dam == me.Archetype.Clone.Dam:
		ac.Write("Your weapon suddenly seems sharper!")
		me.Dam = 2 * me.Archetype.Clone.Dam
		me.Weight = me.Archetype.Clone.Weight
		me.LastSP = me.Archetype.Clone.LastSP
elif (r <= 0.03):
	if me.LastSP == me.Archetype.Clone.LastSP and me.LastSP != 0:
		ac.Write("Your weapon suddenly seems easier to handle!")
		me.LastSP = me.Archetype.Clone.LastSP - 1
		me.Dam = me.Archetype.Clone.Dam
		me.Weight = me.Archetype.Clone.Weight
elif (r <= 0.04):
	ac.Write("Your weapon suddenly seems colder!")
	me.AttackType = Crossfire.AttackType.COLD + Crossfire.AttackType.PHYSICAL
elif (r <= 0.05):
	ac.Write("Your weapon suddenly seems warmer!")
	me.AttackType=Crossfire.AttackType.FIRE + Crossfire.AttackType.PHYSICAL
elif (r <= 0.06):
	ac.Write("Your weapon suddenly emits sparks!")
	me.AttackType=Crossfire.AttackType.ELECTRICITY + Crossfire.AttackType.PHYSICAL
