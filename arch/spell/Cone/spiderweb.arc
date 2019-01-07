# below is the cone effect for the spider web.
# all it really does is drop the spider_web2 archs, which
# then hamper movement.
Object spiderweb_cone
name spider web
other_arch spider_web2
face spider_web.111
move_on walk fly_low
wc -90
move_type fly_low
type 102
subtype 7
speed 1
no_pick 1
end
#
Object spider_web2
name spider web
face spider_web.111
animation spider_web
no_pick 1
move_slow walk
move_slow_penalty 7
material 128
anim_speed 10
client_anim_random 1
end
