# a specific type of spell effect (cone) which operates
# as a ground object.
Object lava
type 102
subtype 7
level 1
walk_on 1
wc -30
name lava
face lava.111
anim
lava.111
lava.112
lava.113
lava.114
lava.115
mina
smoothface lava.111 lava_S.111
smoothface lava.112 lava_S.112
smoothface lava.113 lava_S.113
smoothface lava.114 lava_S.114
smoothface lava.115 lava_S.115
smoothlevel 28
color_fg yellow
color_bg red
speed 0.2
no_pick 1
attacktype 4
duration 60
dam 3
is_floor 1
editable 8
move_block boat swim
end
#
Object permanent_lava
type 102
subtype 7
level 1
walk_on 1
wc -30
lifesave 1
name lava
face lava.111
anim
lava.111
lava.112
lava.113
lava.114
lava.115
mina
color_fg yellow
color_bg red
speed 0.2
no_pick 1
smoothlevel 28
smoothface lava.111 lava_S.111
smoothface lava.112 lava_S.112
smoothface lava.113 lava_S.113
smoothface lava.114 lava_S.114
smoothface lava.115 lava_S.115
attacktype 4
hp 1
dam 3
editable 8
is_floor 1
move_block boat swim
end
