# vitriol is the 'bullet' form of vitriol.
Object vitriol
name vitriol
other_arch vitriol_splash
type 102
subtype 5
face v_splash.111
animation vitriol_splash
is_turnable 0
walk_on 1
fly_on 1
glow_radius 2
speed 1
flying 1
no_pick 1
attacktype 64
editable 0
end
#
# vitriol splash is the cone form the bullet turns into
#
Object vitriol_splash
name vitriol splash
other_arch vitriol_pool
type 102
subtype 7
face v_splash.111
anim
v_splash.111
v_splash.112
mina
is_turnable 1
walk_on 1
fly_on 1
glow_radius 2
speed 1
wc -30
flying 1
no_pick 1
editable 0
end
#
# vitriol pool is what the splash above drops.
# Note that this is put unchanged onto the map.
Object vitriol_pool
type 102
subtype 7
level 1
walk_on 1
wc -30
stand_still 1
name vitriol pool
face v_pool.111
anim
v_pool.111
v_pool.112
mina
speed 0.2
no_pick 1
attacktype 64
duration 30
dam 15
editable 8
end
