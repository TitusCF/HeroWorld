import Crossfire
import random

me = Crossfire.WhoAmI()
ac = Crossfire.WhoIsActivator()
r  = random.random()

# Event is called before object is applied, so changing our properties just
# before it's actually applied instead of when removed
#
# To prevent insane stats like (Dex+127)(Con+85)(Int+57) (see bug #2369654) we
# limit allowed range to +/- 2 for each stat. If we don't change due to these
# limits, we will not fall back to next (to make it harder to get a "perfect"
# ring), but instead just do nothing.
if (me.Applied == 0):
	if   (r <= 0.01):
		if me.Dex < 2:
			me.Cursed= 1
			me.Dex = me.Dex + 1
			me.Identified=0
	elif (r <= 0.02):
		if me.Int < 2:
			me.Cursed= 1
			me.Int = me.Int + 1
			me.Identified=0
	elif (r <= 0.03):
		if me.Con < 2:
			me.Cursed= 1
			me.Con = me.Con + 1
			me.Identified=0
	elif (r >= 0.97):
		if me.Dex > -2:
			me.Cursed= 1
			me.Dex = me.Dex - 1
			me.Identified=0
	elif (r >= 0.98):
		if me.Int > -2:
			me.Cursed= 1
			me.Int = me.Int - 1
			me.Identified=0
	elif (r >= 0.99):
		if me.Con > -2:
			me.Cursed= 1
			me.Con = me.Con - 1
			me.Identified=0
