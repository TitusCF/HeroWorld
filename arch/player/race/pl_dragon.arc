Object pl_dragon
name fire hatchling
race dragon
randomitems dragon_player_items
face pl_dragon_r.151
msg
Skills: Clawing, Levitate, Woodsman, Pyromancy
Resistances: None
Attunements: None
Special: Can not use weapons or most armor.  Natural armor improves over time

Description:
Dragons are completely different than any other race. Their vast size and bizarre body prevents them both from wearing armour and wielding weapons -- tools that they disdain in any case. Instead, they are gifted with the ability to evolve and grow stronger by eating the flesh of their defeated foes.
Over the years, their dragonhide hardens and it can provide better protection than the best armour. Instead of wielding weapons, they use their sharp and lethal claws in combat. Moreover, dragons have a natural talent for magic.
Dragons are very interested in the lore of the elements and usually choose to specialize. While focusing their metabolism on a certain element, they can gain various new abilities -- including new spells, enhanced claws and more. Eventually, a dragon is able to evolve from the hatchling stage into a grown-up terrifying ancient
dragon.
endmsg
animation pl_dragon_r
is_animated 1
anim_speed -1
type 1
level 1
ac 5
wc 21
dam 10
alive 1
weight 70000
food 999
speed 1
attacktype 1
can_use_shield 0
can_use_armour 0
can_use_weapon 0
Str 5
Dex 0
Con 6
Wis -8
Int -3
Pow 5
# can't use bows anymore with this, so let him use cloaks,
# bracers, and girdles.
body_range 1
body_arm 0
body_neck 1
body_skill 1
body_finger 2
body_shoulder 1
body_wrist 2
body_waist 1
maxhp 30
maxsp 30
maxgrace 30
# Key/value list
race_choice_1 dragon_ability_force_fire dragon_ability_force_cold dragon_ability_force_electricity dragon_ability_force_poison
race_choice_description_1 Choose a dragon focus
end
Object dragon_skin_force
name dragon skin
invisible 1
type 114
face blank.111
applied 1
no_drop 1
end
Object dragon_ability_force
name dragon ability
title fire hatchling
invisible 1
type 114
exp 2
face blank.111
neutral 1
no_drop 1
end
# These are used during character creation - presented
# as choice to player to choose one of these.
Object dragon_ability_force_fire
name Fire hatchling focus
title fire hatchling
invisible 1
type 10
exp 2
neutral 1
no_drop 1
face pl_dragon_r.151
animation pl_dragon_r
is_animated 1
anim_speed -1
auto_apply 1
end
#
Object dragon_ability_force_electricity
name Electricity hatchling focus
title electricity hatchling
invisible 1
type 10
exp 3
neutral 1
no_drop 1
face pl_dragon_blue.151
animation pl_dragon_blue
is_animated 1
anim_speed -1
auto_apply 1
end
#
Object dragon_ability_force_cold
name Cold hatchling focus
title cold hatchling
invisible 1
type 10
exp 4
neutral 1
no_drop 1
face pl_dragon_bl.151
animation pl_dragon_bl
is_animated 1
anim_speed -1
auto_apply 1
end
#
Object dragon_ability_force_poison
name Poison hatchling focus
title poison hatchling
invisible 1
type 10
exp 10
neutral 1
no_drop 1
face pl_dragon_g.151
animation pl_dragon_g
is_animated 1
anim_speed -1
auto_apply 1
end
