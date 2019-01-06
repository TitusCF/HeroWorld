# Summoners need a first level spell, and this is it - make it
# so most of the values don't scale - otherwise, this could be too
# powerful at higher levels.
Object spell_lesser_summon_golem
name summon lesser golem
name_pl summon lesser golem
face spell_summoner.111
level 1
sp 3
casting_time 10
path_attuned 64
other_arch golem
dam 10
duration 75
duration_modifier 5
maxsp 15
type 101
subtype 12
value 10
attacktype 1
no_drop 1
invisible 1
skill summoning
msg
Lesser golem summons a magical creature that
does the caster's wishes.  The caster can
have it attack other creatures, bash down
doors, detonate runes, etc.  The golem has a
finite lifetime, and this life is shortened
any time it takes damage.
endmsg
end
