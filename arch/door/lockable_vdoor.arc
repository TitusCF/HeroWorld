Object lockable_vdoor
name unlocked door
move_block 0
face unlocked_vwooddoor.111
randomitems lockable_door
hp 400
exp 100
ac 20
type 20
material 2
no_pick 1
level 1
lockable 1
other_arch lockable_vdoor_locked
msg
This door is locked.
endmsg
end
Object lockable_vdoor_locked
name locked door
face locked_vwooddoor.111
randomitems lockable_door
hp 400
exp 100
ac 20
move_block all
alive 1
type 20
material 2
no_pick 1
level 1
lockable 1
other_arch lockable_vdoor
msg
This door is locked.
endmsg
end
