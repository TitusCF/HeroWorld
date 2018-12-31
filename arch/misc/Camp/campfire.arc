Object campfire
face campfire.111
animation campfire
type 102
subtype 7
level 1
move_on walk
lifesave 1
wc -30
dam 3
attacktype 4
speed -0.2
glow_radius 2
no_pick 1
end

Object campfire_talking
name talking campfire
randomitems campfire_talking
face campfire.111
animation campfire
type 73
glow_radius 2
no_pick 1
anim_speed 3
client_anim_random 1
is_animated 1
end

Object campfire_say
type 116
subtype 6
title Python
slaying /python/items/campfire_say.py
end
Object campfire_timer
type 116
subtype 12
title Python
slaying /python/items/campfire_timer.py
end
