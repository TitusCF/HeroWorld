FIG = fig/earrow.gif fig/nearrow.gif fig/sarrow.gif fig/swarrow.gif\
	fig/narrow.gif fig/nwarrow.gif fig/searrow.gif fig/warrow.gif\
	fig/screen_dia.gif fig/book.gif fig/stairdown.gif fig/stairup.gif

EXTRA_DIST = handbook.shtml chap1.html chap2.shtml chap3.shtml\
	chap4.shtml chap5.shtml chap6.shtml chap7.html appA.html appB.html\
 	appC.shtml bonus-extract char-extract spellpath-extract\
	skills2-extract empty.pbm statskmod-extract treas1-extract\
	treas2-extract skills-extract items-extract generic-extract\
	sorter levels-extract gods-extract $(FIG)
#
# This are automatically generated files
#
HTML = gods.html appC.html stats.gif potion.html flesh.html food.html rings.html \
	books.html money.html shopmat.html shop.html chap6.html lockpicks.html \
	statskmod.html version.html skill_stat.html chap5.html runeshock.html \
	runemark.html runefire.html runefrost.html runegen.html runedeath.html \
	runeblast.html cauldron.html altar.html spellpath.html chap4.html \
	corpse.html gravestone.html wands.html talisman.html sign.html\
	scrolls.html savebed.html rods.html potions.html pit.html levels.html \
	keys.html horns.html holysymbol.html handbutt.html guild.html \
	gatedoor.html fr_wall.html foodone.html dir_spin.html br_wall.html \
	bows.html chap3.html skills.html equip.html stats.html char.html \
	bonus.html chap2.html handbook.html

HANDBOOK = handbook.html chap1.html chap2.html chap3.html chap4.html chap5.html chap6.html appC.html

CLEANFILES = items .bmaps in_monput in_items in_spells in_stats in_equip \
	*x*.ppm tmp.ppm work.ppm in_char items *.png $(HTML)

CROSSBIN = ../../server/crossfire-server
INCDIR = $(top_srcdir)/include
DTOP = $(top_srcdir)/$(PACKAGE)-doc
PERL = @PERL@

handbook: $(HANDBOOK)

dump_switches:
	@$(RM) dump_switches
	@$(CROSSBIN) -m >/dev/null 2>&1 && touch dump_switches ||	(echo "You need to define DUMP_SWITCHES in include/config.h and";	echo "recompile before you can create the spoilers.")

	@$(MAKE) $(MFLAGS) check_ok

check_ok: dump_switches
	@$(RM) dump_switches

.bmaps: $(pkgdatadir)/archetypes
	$(RM) *.gif
	touch .bmaps

items: .bmaps in_items ../scripts/makeps.pl items.gif
	$(PERL) ../scripts/makeps.pl output=png libdir=$(top_srcdir)/lib archdir=$(top_srcdir)/lib/ size=0.4 input=in_items  inarch=$(pkgdatadir)/archetypes > items

in_items: items-extract $(pkgdatadir)/archetypes $(top_srcdir)/common/living.c
	$(AWK) -v living_c=$(top_srcdir)/common/living.c 		-f items-extract $(pkgdatadir)/archetypes > in_items

handbook.html: version.html handbook.shtml
	$(PERL) $(srcdir)/../include_html.pl handbook.shtml handbook.html

chap2.html: bonus.html char.html equip.html skills.html chap2.shtml
	$(PERL) $(srcdir)/../include_html.pl chap2.shtml chap2.html

bonus.html:
	$(AWK) -v living_c=../../common/living.c 	-f bonus-extract > bonus.html

in_char: char-extract $(pkgdatadir)/archetypes $(top_srcdir)/common/living.c
	$(AWK) -v living_c=$(top_srcdir)/common/living.c 		-f char-extract $(pkgdatadir)/archetypes > in_char

char.html: .bmaps in_char ../scripts/makeps.pl characters.gif
	$(PERL) ../scripts/makeps.pl output=png libdir=$(top_srcdir)/lib archdir=$(top_srcdir)/lib/ size=0.4 input=in_char inarch=$(pkgdatadir)/archetypes > char.html

in_stats: stats-extract $(pkgdatadir)/archetypes $(top_srcdir)/common/living.c
	$(AWK) -v living_c=$(top_srcdir)/common/living.c 		-f stats-extract $(pkgdatadir)/archetypes > in_stats

stats.html: .bmaps in_stats ../scripts/makeps.pl stats.gif
	$(PERL) ../scripts/makeps.pl output=png libdir=$(top_srcdir)/lib archdir=$(top_srcdir)/lib/ size=0.4 input=in_stats archdir=$(pkgdatadir)/archetypes > stats.html

equip.html: in_equip treas2-extract
	$(AWK) -v eqitems=in_equip -f treas2-extract 	../../lib/treasures > equip.html

in_equip: treas1-extract $(pkgdatadir)/archetypes $(top_srcdir)/common/living.c
	$(AWK) -v living_c=$(top_srcdir)/common/living.c 		-f treas1-extract $(pkgdatadir)/archetypes | sort -d > in_equip

skills.html: in_equip skills-extract
	$(AWK) -v eqitems=in_equip -f skills-extract 	../../lib/treasures > skills.html

chap3.html: chap3.shtml bows.html br_wall.html dir_spin.html foodone.html 	fr_wall.html gatedoor.html guild.html handbutt.html 	holysymbol.html horns.html keys.html pit.html potions.html 	rods.html savebed.html scrolls.html sign.html corpse.html 	talisman.html wands.html levels.html gravestone.html
	$(PERL) $(srcdir)/../include_html.pl chap3.shtml chap3.html

bows.html: items generic-extract
	egrep -e "^14 " items | $(AWK) -F'&'  -f generic-extract | sort > bows.html

br_wall.html: items sorter
	egrep -e "^45 " items | $(AWK) -F'&' -f generic-extract | sort > br_wall.html

dir_spin.html: items sorter
	egrep -e "^90 " items | $(AWK) -F'&' -f generic-extract | sort > dir_spin.html
	egrep -e "^112 " items | $(AWK) -F'&' -f generic-extract | sort >> dir_spin.html

foodone.html: items sorter
	egrep -e "^6 " items | $(AWK) -F'&'  -f generic-extract | grep "food" > foodone.html

fr_wall.html: items sorter
	egrep -e "^62 " items | $(AWK) -F'&' -f generic-extract | sort > fr_wall.html

gatedoor.html: items sorter
	egrep -e "^20 " items | $(AWK) -F'&' -f generic-extract | sort > gatedoor.html
	egrep -e "^23 " items | $(AWK) -F'&' -f generic-extract | sort >> gatedoor.html
	egrep -e "^91 " items | $(AWK) -F'&' -f generic-extract | sort >> gatedoor.html

guild.html: items generic-extract
	egrep -e "^66 " items | grep "guild" | $(AWK) -F'&'  	-f generic-extract | sort > guild.html

handbutt.html: items sorter
	egrep -e "^92 " items | $(AWK) -F'&' -f generic-extract | sort > handbutt.html
	egrep -e "^93 " items | $(AWK) -F'&' -f generic-extract | sort >> handbutt.html

holysymbol.html: items generic-extract
	egrep -e "^43 " items | grep "holy" | $(AWK) -F'&'  	-f generic-extract | sort > holysymbol.html

horns.html: items sorter
	egrep -e "^35 " items | $(AWK) -F'&'  -f generic-extract | sort > horns.html

keys.html: items sorter
	egrep -e "^21 " items | $(AWK) -F'&' -f generic-extract | sort > keys.html
	egrep -e "^24 " items | $(AWK) -F'&' -f generic-extract | sort >> keys.html

levels.html:
	$(AWK) -v living_c=../../common/living.c 	-f levels-extract > levels.html

pit.html: items sorter
	egrep -e "^95 " items | $(AWK) -F'&' -f generic-extract | sort > pit.html

potions.html: items sorter
	egrep -e "^5 " items | $(AWK) -F'&'  -f generic-extract | sort > potions.html

rods.html: items sorter
	egrep -e "^3 " items | $(AWK) -F'&'  -f generic-extract | sort > rods.html

savebed.html: items sorter
	egrep -e "^106 " items | $(AWK) -F'&'  -f generic-extract | sort > savebed.html

scrolls.html: items sorter
	egrep -e "^111 " items | $(AWK) -F'&'  -f generic-extract | sort > scrolls.html

sign.html: items sorter
	egrep -e "^98 " items | $(AWK) -F'&' -f generic-extract | sort > sign.html

talisman.html: items generic-extract
	egrep -e "^43 " items | grep "talisman" | $(AWK) -F'&'  	-f generic-extract | sort > talisman.html

wands.html: items sorter
	egrep -e "^109 " items | $(AWK) -F'&'  -f generic-extract | sort > wands.html

gravestone.html: items sorter
	egrep -e "^38 " items | $(AWK) -F'&' -f generic-extract | sort > gravestone.html

corpse.html: items sorter
	egrep -e "^157 " items | $(AWK) -F'&' -f generic-extract | sort > corpse.html

chap4.html: chap4.shtml spellpath.html altar.html cauldron.html 	runefire.html runefrost.html runeblast.html runeshock.html	 runedeath.html  runegen.html runemark.html
	$(PERL) $(srcdir)/../include_html.pl chap4.shtml chap4.html

spellpath.html:
	$(AWK) -v living_c=../../include/spellist.h 	-f spellpath-extract |sort > spellpath.html

altar.html: items sorter
	egrep -e "^18 " items | $(AWK) -F'&' -f generic-extract | sort > altar.html

cauldron.html: items sorter
	egrep -e "^122 " items | grep "cauldron" | 		$(AWK) -F'&' -f generic-extract | sort > cauldron.html

runeblast.html: items sorter
	egrep -e "^154 " items | grep "rune-blast" | 	$(AWK) -F'&' -f generic-extract | sort > runeblast.html

runedeath.html: items sorter
	egrep -e "^154 " items | grep "rune-death" | 	$(AWK) -F'&' -f generic-extract | sort > runedeath.html

runegen.html: items sorter
	egrep -e "^154 " items | grep "generic-rune" | 	$(AWK) -F'&' -f generic-extract | sort > runegen.html

runefrost.html: items sorter
	egrep -e "^154 " items | grep "rune-frost." | 	$(AWK) -F'&' -f generic-extract | sort > runefrost.html

runefire.html: items sorter
	egrep -e "^154 " items | grep "rune-fire." | 	$(AWK) -F'&' -f generic-extract | sort > runefire.html

runemark.html: items sorter
	egrep -e "^98 " items | grep "rune-mark." | 	$(AWK) -F'&' -f generic-extract | sort > runemark.html

runeshock.html: items sorter
	egrep -e "^154 " items | grep "rune-shock." | 	$(AWK) -F'&' -f generic-extract | sort > runeshock.html

chap5.html: chap5.shtml skill_stat.html statskmod.html lockpicks.html
	$(PERL) $(srcdir)/../include_html.pl chap5.shtml chap5.html

skill_stat.html: skills2-extract $(CROSSBIN)
		$(AWK) -F, -v crosscmd="$(CROSSBIN) -m5" 			-f skills2-extract | sort > skill_stat.html

version.html: $(CROSSBIN)
	$(CROSSBIN) -v 2>&1 | grep "^This is" | 		cut -d" " -f3- > version.html

statskmod.html:
	$(AWK) -v living_c=../../server/skill_util.c 	-f statskmod-extract > statskmod.html

lockpicks.html: items sorter
	egrep -e "^43 " items | grep "lockpicks" | 	$(AWK) -F'&' -f generic-extract | sort > lockpicks.html

chap6.html: chap6.shtml shop.html shopmat.html money.html books.html 	scrolls.html potions.html wands.html rods.html horns.html 	rings.html food.html flesh.html potion.html
	$(PERL) $(srcdir)/../include_html.pl chap6.shtml chap6.html

shop.html: items sorter
	egrep -e "^66 " items | grep "store" | 	$(AWK) -F'&' -f generic-extract | sort > shop.html

shopmat.html: items sorter
	egrep -e "^69 " items | 	$(AWK) -F'&' -f generic-extract | sort > shopmat.html

money.html: items sorter
	egrep -e "^36 " items | 	$(AWK) -F'&' -f generic-extract | sort > money.html

books.html: items sorter
	egrep -e "^8 " items | 	$(AWK) -F'&' -f generic-extract | sort > books.html

rings.html: items sorter
	egrep -e "^70 " items | 	$(AWK) -F'&' -f generic-extract | sort > rings.html

food.html: items sorter
	egrep -e "^6 " items | 	$(AWK) -F'&' -f generic-extract | sort > food.html

flesh.html: items sorter
	egrep -e "^72 " items | 	$(AWK) -F'&' -f generic-extract | sort > flesh.html

potion.html: items sorter
	egrep -e "^5 " items | grep "potiongen" | 	$(AWK) -F'&' -f generic-extract | sort > potion.html

stats.gif items.gif monsters.gif spells.gif characters.gif:
	touch $@

appC.html: appC.shtml gods.html
	$(PERL) $(srcdir)/../include_html.pl appC.shtml appC.html

gods.html: gods-extract $(CROSSBIN)
	$(AWK) -F, -v crosscmd="$(CROSSBIN) -m8" -f gods-extract > gods.html

archive: $(HANDBOOK)
	$(mkinstalldirs) $(DTOP)/handbook-html/fig
	cp $(HANDBOOK) $(DTOP)/handbook-html
	cp $(FIG) $(DTOP)/handbook-html/fig
	cp *.gif $(DTOP)/handbook-html
