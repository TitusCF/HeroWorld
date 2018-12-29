Object diamond_converter
name drop gold to buy diamonds
other_arch gem
slaying goldcoin
food 40
type 103
no_pick 1
face conv.111
walk_on 1
editable 64
visibility 100
magicmap light_blue
end
Object pearl_converter
name drop gold to buy pearls
other_arch pearl
slaying goldcoin
type 103
food 5
no_pick 1
face conv.111
walk_on 1
editable 64
visibility 100
end
Object ruby_converter
name drop gold to buy rubies
other_arch ruby
slaying goldcoin
food 20
type 103
no_pick 1
face conv.111
walk_on 1
editable 64
visibility 100
end
Object silver_converter
name convert silver into gold
other_arch goldcoin
slaying silvercoin
food 10
type 103
no_pick 1
face conv.111
walk_on 1
editable 64
visibility 100
end
Object gold_converter
name convert gold into platinum
other_arch platinacoin
slaying goldcoin
food 5
type 103
no_pick 1
face conv.111
walk_on 1
editable 64
visibility 100
end
Object platinum_converter
name convert platinum into gold
other_arch goldcoin
slaying platinacoin
food 1
sp 5
type 103
no_pick 1
face conv.111
walk_on 1
editable 64
visibility 100
end
# DON'T USE THIS OBJECT YET
# it needs some server code which is under discussion
# a character having amberium or jade coins without the server code in
# place would be able to simply walk out of shops with stuff, without
# actually paying anything.
Object platinum_converter2
name convert platinum into jade
other_arch jadecoin
slaying platinacoin
food 100
type 103
no_pick 1
face conv.111
walk_on 1
editable 64
visibility 100
end
# DON'T USE THIS OBJECT YET
# it needs some server code which is under discussion
# a character having amberium or jade coins without the server code in
# place would be able to simply walk out of shops with stuff, without
# actually paying anything.
Object jade_converter
name convert jade into amberium
other_arch ambercoin
slaying jadecoin
food 100
type 103
no_pick 1
face conv.111
walk_on 1
editable 64
visibility 100
end
