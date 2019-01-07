Object spell_glyph
anim_suffix spellcasting
name glyph
name_pl glyph
face spell_glyph.111
type 101
subtype 2
no_drop 1
invisible 1
level 12
value 120
grace 5
casting_time 15
skill praying
path_attuned 2048
msg
Creates a special rune that may be used to encapsulate another spell.  When casting the spell, the caster specifies a praying spell to encapsulate in the glyph.  When the glyph is triggered, the specified spell is cast on the target.

An example of how to create a glyph is:
 cast glyph summon cult monsters
endmsg
end
