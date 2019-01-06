Object oil_sea
face oil_sea.111
anim
oil_sea.111
oil_sea.112
oil_sea.113
oil_sea.114
oil_sea.113
oil_sea.112
mina
name oil ocean
name_pl oil ocean
magicmap black
smoothlevel 16
smoothface oil_sea.111 oil_sea_S.111
smoothface oil_sea.112 oil_sea_S.112
smoothface oil_sea.113 oil_sea_S.113
smoothface oil_sea.114 oil_sea_S.114
speed 0.1
no_pass 1
no_pick 1
is_water 1
editable 8
is_floor 1
move_block all -boat
client_anim_sync 1
end

Object oil_sea_ultra_viscous
#So thick you can almost walk on it (To be used where on needs walk-on-able/nonblocking oil)
#(ex: some random maps). For most applications you should use the regular oil_sea
#which will be swimable as water is (but much slower swiming ofcourse)
face oil_sea.111
anim
oil_sea.111
oil_sea.112
oil_sea.113
oil_sea.114
oil_sea.113
oil_sea.112
mina
name ultra viscous oil
name_pl ultra viscous oil
magicmap black
smoothlevel 16
smoothface oil_sea.111 oil_sea_S.111
smoothface oil_sea.112 oil_sea_S.112
smoothface oil_sea.113 oil_sea_S.113
smoothface oil_sea.114 oil_sea_S.114
speed 0.01
no_pass 0
no_pick 1
is_water 1
editable 8
is_floor 1
client_anim_sync 1
end

