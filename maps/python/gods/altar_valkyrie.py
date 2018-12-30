"""Altar of Valkyrie
Followers of Valkyrie don't get any praying spells, because Valkyrie hates magic.
Instead, they gain experience by combat bravery; and the way they prove that is
by bringing the flesh of dead enemies as a sacrifice in Her altar.

Of course, the script only activates for followers of Valkyrie, and only runs for
sacrifices of type FLESH.

Then, it can handle each in two ways:

- Ideally, all items will have Exp stored.  In this case, you'll get 1/5 of that
  Exp, with a bonus if it's a head or heart.

- Otherwise, we'll use the Level and resistances to estimate how hard it was to
  kill the monster.  In fact, I'm not at all certain the algorithm used to
  estimate is reasonable at all for higher levels...  but then again, I'm not
  sure it's still necessary either, so feel free to remove it :-)
"""

import Crossfire

def accept(description):
    pl.Write('Valkyrie accepts your %s sacrifice!' % description)

# XXX: need to expose NROFATTACKS to Python

altar = Crossfire.WhoAmI()
pl = Crossfire.WhoIsActivator()
praying = pl.CheckArchInventory('skill_praying')
if praying and praying.Title == 'Valkyrie':

    # accept sacrifice
    obj = altar.Above
    while obj:
        if obj.Unpaid:
            pl.Write('Valkyrie scorns your stolen sacrifice!')
            break
        if obj.Type & 0xffff == Crossfire.Type.FLESH:
            level_factor = 0
            part_factor = 1

            if obj.Level < praying.Level / 2:
                if obj.Exp:
                    pl.Write('Valkyrie grudgingly accepts your pathetic sacrifice!')
                else:
                    pl.Write('Valkyrie scorns your pathetic sacrifice!')
            elif obj.Level < praying.Level:
                accept('poor')
                level_factor = 0.5
            elif obj.Level < praying.Level * 1.5:
                accept('modest')
                level_factor = 1
            elif obj.Level < praying.Level * 2:
                accept('adequate')
                level_factor = 1.5
            elif obj.Level < praying.Level * 5:
                accept('devout')
                level_factor = 2
            else:
                accept('heroic')
                level_factor = 2.5

            # heads and hearts are worth more.  Because.
            if obj.Name.endswith('head') or obj.Name.endswith('heart'):
                part_factor = 1.5

            if obj.Exp:
                # obj has stored exp, use it
                value = obj.Exp / 5 * part_factor

            else:
                # no stored exp, estimate
                # flesh with lots of resists is worth more
                res = 0
                for at in range(26):  # XXX should be NROFATTACKS
                    res += obj.GetResist(at)

                value = max(res, 10) * level_factor * part_factor

            if obj.Quantity > 1:
                obj.Quantity -= 1
            else:
                obj.Remove()
            pl.AddExp(value, 'praying')
            break
        obj = obj.Above
