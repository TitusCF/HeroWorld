# These archetypes are presently modeled after the contents found in pit.arc in
# the arch/trunk/connect/Hole directory.  The difference is that this archetype
# is a 64 x 64 bit multi-tile HOLE.
#
# This animation shold work as expected.
#
Object manhole_closed_1
name manhole
type 94
activate_on_push 1
activate_on_release 1
no_pick 1
face manhole.114
animation manhole
is_animated 0
wc 4
end
More
Object manhole_closed_1a
face manhole.114
animation manhole
x 1
end
More
Object manhole_closed_1b
face manhole.114
animation manhole
y 1
end
More
Object manhole_closed_1c
face manhole.114
animation manhole
x 1
y 1
end
#
# This animation should work.
#
Object manhole_open_1
name manhole
type 94
activate_on_push 1
activate_on_release 1
no_pick 1
face manhole.111
animation manhole
is_animated 0
move_on walk
wc 0
maxsp 1
end
More
Object manhole_open_1a
face manhole.111
animation manhole
maxsp 1
x 1
end
More
Object manhole_open_1b
face manhole.111
animation manhole
maxsp 1
y 1
end
More
Object manhole_open_1c
face manhole.111
maxsp 1
animation manhole
x 1
y 1
end
