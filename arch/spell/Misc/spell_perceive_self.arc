# Examples:
# ---
# You worship Gaea
# You have:
# (lightning fast movement)(wear
# armour)(grace+3)(regeneration+3)(luck+1)(Attacks:
# physical)(Attuned: Protection,
# Summoning)(Denied: Wounding,
# # Death)(armour +15)(resist electricity
# +15)(resist drain +100)(resist fear
# -100)(resist depletion +100)(resist
# death +100)(resist chaos +10)
# ---
# You worship Valriel
# You have:
# (lightning fast movement)(wield
# weapon)(wear armour)(Attacks:
# physical)(Attuned: Protection,
# Mind)(Denied: Wounding, Death)(armour
# +26)(resist fire +30)(resist electricity
# +30)(resist confusion +20)(resist fear
# +100)(resist blindness +100)
# Your strength is depleted by 2
# You glow in the dark.
# ---
Object spell_perceive_self
anim_suffix spellcasting
name perceive self
name_pl perceive self
face spell_perceive_self.111
level 4
grace 5
casting_time 1
path_attuned 8192
skill praying
type 101
subtype 17
value 20
no_drop 1
invisible 1
msg
Displays information about the caster including: God worshipped; attributes, bonuses; attacks; paths attuned, denied, and repelled; protections; stat depletions, special characteristics.

Much of the information is able to be found other ways, but the spell is still useful at times.
endmsg
end
