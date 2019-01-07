# vitriol is the 'bullet' form of vitriol.
Object vitriol
name vitriol
other_arch vitriol_splash
type 102
subtype 5
face v_splash.111
animation vitriol_splash
is_turnable 0
move_on walk fly_low
glow_radius 2
speed 1
move_type fly_low
no_pick 1
attacktype 64
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
animation vitriol_splash
is_turnable 1
move_on walk fly_low
glow_radius 2
speed 1
wc -30
move_type fly_low
no_pick 1
end
#
# vitriol pool is what the splash above drops.
# Note that this is put unchanged onto the map.
Object vitriol_pool
type 102
subtype 7
level 1
move_on walk
wc -30
stand_still 1
name vitriol pool
face v_pool.111
animation vitriol_pool
speed 0.2
no_pick 1
attacktype 64
duration 30
dam 15
end
