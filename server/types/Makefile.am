AUTOMAKE_OPTIONS = subdir-objects

noinst_LIBRARIES = libtypes.a
libtypes_a_SOURCES = altar/altar.c \
armour_improver/armour_improver.c \
arrow/arrow.c \
blindness/blindness.c \
book/book.c \
button/button.c \
cf_handle/cf_handle.c \
check_inv/check_inv.c \
clock/clock.c \
container/container.c \
converter/converter.c \
creator/creator.c \
deep_swamp/deep_swamp.c \
detector/detector.c \
director/director.c \
dragon_focus/dragon_focus.c \
duplicator/duplicator.c \
exit/exit.c \
food/food.c \
gate/gate.c \
hole/hole.c \
identify_altar/identify_altar.c \
lamp/lamp.c \
lighter/lighter.c \
marker/marker.c \
mood_floor/mood_floor.c \
peacemaker/peacemaker.c \
pedestal/pedestal.c \
player_changer/player_changer.c \
player_mover/player_mover.c \
poison/poison.c \
poisoning/poisoning.c \
potion/potion.c \
power_crystal/power_crystal.c \
savebed/savebed.c \
scroll/scroll.c \
shop_inventory/shop_inventory.c \
shop_mat/shop_mat.c \
sign/sign.c \
skillscroll/skillscroll.c \
spell_effect/spell_effect.c \
spellbook/spellbook.c \
spinner/spinner.c \
teleporter/teleporter.c \
thrown_object/thrown_object.c \
transport/transport.c \
trap/common_trap.c \
trap/trap.c \
trap/rune.c \
trapdoor/trapdoor.c \
treasure/treasure.c \
trigger/trigger.c \
trigger_altar/trigger_altar.c \
trigger_button/trigger_button.c \
trigger_pedestal/trigger_pedestal.c \
weapon_improver/weapon_improver.c \
common/common_apply.c \
common/describe.c \
common/projectile.c \
legacy/apply.c \
legacy/legacy_describe.c \
legacy/process.c


AM_CPPFLAGS = -I$(top_srcdir)/include -DDATADIR=\"$(pkgdatadir)\" -DCONFDIR=\"$(pkgconfdir)\" \
	      -DLIBDIR=\"$(pkglibdir)\" -DLOCALDIR=\"$(pkgstatedir)\" \
	      -DPLUGIN_SUFFIX=\".so\"

proto:
	cproto -D__CEXTRACT__ -o ../include/typesproto.h $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(SOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 ../include/typesproto.h
