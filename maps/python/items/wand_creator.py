# Written by James W. Bennett in 2020
# Released as GPL
# Concept was Titus', for their "HeroWorld" server
# 
# Takes a spell from a spellbook and puts it into a wand.
# 
# USING THIS SCRIPT:
# 1. Add to the wandworker table's inventory an "event_apply" object.  The plugin name should be "Python", and it should point to this script.
# 2. Provide a level 0 wand/staff with no spells in it, an identified spellbook, and a glowing crystal.
# 3. Say something.
# 4. Note: Wands at level 0 with 0 charges can't actually cast anything.  This is for use with a wand-upgrades system, which handles level and charges.
# 
# CHANGING THIS SCRIPT:
# - If you want the script to handle higher starting level wands, do something like '''if tableinv.Name.startswith("wand (lvl")''' instead of the current wand check.
# - There's no technical limitation with regards to putting prayers in wands.  We just check for it to conform to Crossfire's normal wand/staff distinction.
# - If you want, rods should be handleable the same way as wands/staves, though they probably also need their mana set
# - Maybe this is better handled via in-game thaumaturgy, but I only know Python.  We could try to simulate a thaumaturgy check and give experience for it in this script, if that's desired.

import Crossfire

scob = Crossfire.WhoAmI()
act = Crossfire.WhoIsActivator()

def create_wand():
	invcount = 0
	tableinv = scob.Inventory
	book_ingred = None
	wand_ingred = None
	crystal_ingred = None
	spell_ingred = None
	
	while tableinv != None:#Counts the number of items in the table, and tries to find each component
		if tableinv.Name in ("wand (lvl 0)", "staff (lvl 0)"):
			if tableinv.Identified == 0:
				act.Message("The wand or staff needs to be identified first.")
			else:
				wand_ingred = tableinv
		if tableinv.Name == "glowing crystal":
			if tableinv.Identified == 0:
				act.Message("The glowing crystal needs to be identified first.")
			else:
				crystal_ingred = tableinv
		if tableinv.Type == 85:
			if tableinv.Identified == 0:
				act.Message("The spellbook needs to be identified first.")
			else:
				book_ingred = tableinv
				bookinv = book_ingred.Inventory
				while bookinv != None:#We also look into the book's inventory for the spell.
					spell_ingred = bookinv
					bookinv = bookinv.Below
		invcount += 1
		tableinv = tableinv.Below
	if invcount > 4: # Runs if there's more item than the event plus three ingredients in the table.
		act.Message("There should only be three items in the table.")
	elif None in (book_ingred, wand_ingred, crystal_ingred, spell_ingred):# Checks if any of the ingredients are missing, including the spell
		act.Message("To create a wand, drop a blank wand or staff, an identified spellbook, and a glowing crystal inside.")
	else:
		if (str(book_ingred.Name).startswith("prayerbook")) ^ (str(wand_ingred.Name).startswith("staff")):#Checks if you're trying to use a staff without a prayerbook, or vice-versa.
			act.Message("Prayers can only be placed in staves, and other spells can only be placed in wands.")
		else:
			spell_ingred.InsertInto(wand_ingred)#Takes the spell from the book and places it directly into the wand
			crystal_ingred.Remove()
			book_ingred.Remove()
			act.Message("You have successfully created a " + str(wand_ingred.Name))

create_wand()