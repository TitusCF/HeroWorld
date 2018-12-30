# if you want output as PDF, change the following lines to pdflatex and pdf respectively.
LATEX = latex
OUTPUT = tex

TEX   =	handbook.tex title.tex chap1.tex chap2.tex chap3.tex chap4.tex\
	chap5.tex chap6.tex chap7.tex appA.tex appB.tex appC.tex

dist_noinst_SCRIPTS = make_index.pl

EXTRA_DIST = Makefile.in bonus-extract char-extract items-extract generic-extract\
 	gods-extract levels-extract local.sty skills-extract skills2-extract\
	spellpath-extract statskmod-extract treas1-extract treas2-extract \
	$(TEX) handbook.idx sorter empty.pbm fig/screen_dia.ps

TABLES=	bonus.tex char.tex equip.tex index.tex gods.tex levels.tex skills.tex\
	skill_stat.tex statskmod.tex spellpath.tex version.tex

ICONS =	altar.tex arm_mail.tex br_wall.tex books.tex bows.tex cauldron.tex \
	corpse.tex dir_spin.tex flesh.tex food.tex foodone.tex fr_wall.tex \
	gatedoor.tex gems.tex gravestone.tex guild.tex holysymbol.tex \
	handbutt.tex horns.tex id_altar.tex keys.tex lockpicks.tex\
	lighter.tex money.tex pit.tex potion.tex potions.tex rings.tex \
	rods.tex runeblast.tex runedeath.tex runefire.tex runefrost.tex\
	runeshock.tex runemark.tex runegen.tex savebed.tex scrolls.tex \
	shops.tex shopmat.tex sign.tex talisman.tex wands.tex

CLEANFILES = $(TABLES) $(ICONS) items .bmaps in_* *.ps *x*.ppm tmp.ppm work.ppm \
	     handbook.toc handbook.lot handbook.dvi handbook.log handbook.aux handbook.idx

CROSSBIN = ../../server/crossfire-server
PERL = @PERL@

handbook: ../handbook.ps

../handbook.ps: handbook.dvi
	dvips -o ../handbook.ps handbook.dvi

handbook.dvi: handbook.aux
	$(LATEX) handbook.tex

handbook.aux: handbook.tex $(TABLES) $(ICONS)
	$(RM) handbook.aux
	$(LATEX) handbook.tex

dump_switches: $(CROSSBIN)
	@$(RM) dump_switches
	@$(CROSSBIN) -m >/dev/null 2>&1 && touch dump_switches ||	(echo "You need to define DUMP_SWITCHES in include/config.h and";	echo "recompile before you can create the handbook.")
	@$(MAKE) $(MFLAGS) check_ok

$(CROSSBIN):
	( cd $(top_srcdir)/server ; make crossfire)

check_ok: dump_switches
	@$(RM) dump_switches

.bmaps: $(top_srcdir)/lib/archetypes
	$(RM) *.ps
	touch .bmaps

index.tex handbook.idx: make_index.pl
	touch handbook.idx
	$(PERL) make_index.pl handbook.idx > index.tex;

items: .bmaps in_items ../scripts/makeps.pl items.ps
	$(PERL) ../scripts/makeps.pl output=$(OUTPUT) libdir=$(top_srcdir)/lib archdir=$(top_srcdir)/lib/ size=0.4  input=in_items inarch=$(pkgdatadir)/archetypes > items

in_items: items-extract $(pkgdatadir)/archetypes
	$(AWK) -f items-extract $(pkgdatadir)/archetypes > in_items

bonus.tex:
	$(AWK) -v living_c=$(top_srcdir)/common/living.c 	-f bonus-extract > bonus.tex

char.tex: .bmaps in_char ../scripts/makeps.pl characters.ps
	$(PERL) ../scripts/makeps.pl output=$(OUTPUT) libdir=$(top_srcdir)/lib archdir=$(top_srcdir)/lib/ size=0.4 input=in_char inarch=$(pkgdatadir)/archetypes > char.tex

in_char: char-extract $(pkgdatadir)/archetypes $(top_srcdir)/common/living.c
	$(AWK) -v living_c=$(top_srcdir)/common/living.c 		-f char-extract $(pkgdatadir)/archetypes > in_char

equip.tex: in_equip treas2-extract
	$(AWK) -v eqitems=in_equip -f treas2-extract $(top_srcdir)/lib/treasures.bld > equip.tex

in_equip: treas1-extract $(pkgdatadir)/archetypes $(top_srcdir)/common/living.c
	$(AWK) -v living_c=$(top_srcdir)/common/living.c 		-f treas1-extract $(pkgdatadir)/archetypes | sort -d > in_equip

gods.tex: gods-extract $(CROSSBIN)
	$(AWK) -F, -v crosscmd="$(CROSSBIN) -m8" -f gods-extract > gods.tex

levels.tex:
	$(AWK) -v living_c=../../common/living.c 	-f levels-extract > levels.tex

skills.tex: in_equip skills-extract
	$(AWK) -v eqitems=in_equip -f skills-extract $(top_srcdir)/lib/treasures.bld > skills.tex

skill_stat.tex: skills2-extract $(CROSSBIN)
		$(AWK) -F, -v crosscmd="$(CROSSBIN) -m5" 			-f skills2-extract | sort -u > skill_stat.tex

statskmod.tex:
	$(AWK) -v living_c=../../server/skill_util.c 	-f statskmod-extract > statskmod.tex

spellpath.tex:
	$(AWK) -v living_c=../../include/spellist.h 	-f spellpath-extract |sort -u > spellpath.tex

version.tex: $(CROSSBIN)
	$(CROSSBIN) -v 2>&1 | grep "^This is" | cut -d" " -f4- > version.tex

sp_item.tex: handbutt.tex gatedoor.tex pit.tex br_wall.tex fr_wall.tex dir_spin.tex

characters.ps items.ps:
	touch $@

cauldron.tex: items sorter
	egrep -e "^122 " items | grep "cauldron" | $(AWK) -F'&' 	-f generic-extract | sort -u > cauldron.tex

guild.tex: items sorter
	egrep -e "^66 " items | grep "guild" | $(AWK) -F'&' 	-f generic-extract | sort -u > guild.tex

holysymbol.tex: items sorter
	egrep -e "^74 " items | grep "holy" | $(AWK) -F'&' 	-f generic-extract | sort -u > holysymbol.tex

lockpicks.tex: items sorter
	egrep -e "^74 " items | grep "lockpicks" | $(AWK) -F'&' 	-f generic-extract | sort -u > lockpicks.tex

potion.tex: items sorter
	egrep -e "^5 " items | grep "potiongen" | $(AWK) -F'&' 	-f generic-extract | sort -u > potion.tex

runeblast.tex: items sorter
	egrep -e "^154 " items | grep "rune-blast" | 	$(AWK) -F'&' -f generic-extract | sort -u > runeblast.tex

runedeath.tex: items sorter
	egrep -e "^154 " items | grep "rune-death" | 	$(AWK) -F'&' -f generic-extract | sort -u > runedeath.tex

runegen.tex: items sorter
	egrep -e "^101 " items | grep -e "penta" | 	$(AWK) -F'&' -f generic-extract | sort -u > runegen.tex
	egrep -e "^101 " items | grep -e "glyph" | 	$(AWK) -F'&' -f generic-extract | sort -u >> runegen.tex

runefrost.tex: items sorter
	egrep -e "^154 " items | grep "rune-frost." | 	$(AWK) -F'&' -f generic-extract | sort -u > runefrost.tex

runefire.tex: items sorter
	egrep -e "^154 " items | grep "rune-fire." | 	$(AWK) -F'&' -f generic-extract | sort -u > runefire.tex

runemark.tex: items sorter
	egrep -e "^98 " items | grep "rune-mark." | 	$(AWK) -F'&' -f generic-extract | sort -u > runemark.tex

runeshock.tex: items sorter
	egrep -e "^154 " items | grep "rune-shock." | 	$(AWK) -F'&' -f generic-extract | sort -u > runeshock.tex

shops.tex: items sorter
	egrep -e "^66 " items | grep "store" | $(AWK) -F'&' 	-f generic-extract | sort -u > shops.tex

talisman.tex: items sorter
	egrep -e "^74 " items | grep "talisman" | $(AWK) -F'&' 	-f generic-extract | sort -u > talisman.tex

altar.tex: items sorter
	egrep -e "^18 " items | $(AWK) -F'&' -f generic-extract | sort -u > altar.tex

arm_mail.tex: items generic-extract
	egrep -e "^16 " items | $(AWK) -F'&' -f generic-extract | 	sort -t'&' -n +4 -5 > arm_mail.tex

br_wall.tex: items sorter
	egrep -e "^45 " items | $(AWK) -F'&' -f generic-extract | sort -u > br_wall.tex

books.tex: items sorter
	egrep -e "^8 " items | $(AWK) -F'&' -f generic-extract | sort -u > books.tex

bows.tex: items generic-extract
	egrep -e "^14 " items | $(AWK) -F'&' -f generic-extract | sort -u > bows.tex

corpse.tex: items sorter
	egrep -e "^157 " items | $(AWK) -F'&' -f generic-extract | sort -u > corpse.tex

dir_spin.tex: items sorter
	egrep -e "^90 " items | $(AWK) -F'&' -f generic-extract | sort -u > dir_spin.tex
	egrep -e "^112 " items | $(AWK) -F'&' -f generic-extract | sort -u >> dir_spin.tex

flesh.tex: items sorter
	egrep -e "^72 " items | $(AWK) -F'&' -f generic-extract | sort -u > flesh.tex

food.tex: items sorter
	egrep -e "^6 " items | $(AWK) -F'&' -f generic-extract | sort -u > food.tex

foodone.tex: items sorter
	egrep -e "^6 " items | $(AWK) -F'&' -f generic-extract | grep "food" > foodone.tex

fr_wall.tex: items sorter
	egrep -e "^62 " items | $(AWK) -F'&' -f generic-extract | sort -u > fr_wall.tex

gatedoor.tex: items sorter
	egrep -e "^20 " items | $(AWK) -F'&' -f generic-extract | sort -u > gatedoor.tex
	egrep -e "^23 " items | $(AWK) -F'&' -f generic-extract | sort -u >> gatedoor.tex
	egrep -e "^91 " items | $(AWK) -F'&' -f generic-extract | sort -u >> gatedoor.tex

gems.tex: items sorter
	egrep -e "^60 " items | $(AWK) -F'&' -f generic-extract | sort -u > gems.tex

gravestone.tex: items sorter
	egrep -e "^98 " items | $(AWK) -F'&' -f generic-extract | grep "gravestone\." > gravestone.tex

handbutt.tex: items sorter
	egrep -e "^92 " items | $(AWK) -F'&' -f generic-extract | sort -u > handbutt.tex
	egrep -e "^93 " items | $(AWK) -F'&' -f generic-extract | sort -u >> handbutt.tex

horns.tex: items sorter
	egrep -e "^35 " items | $(AWK) -F'&' -f generic-extract | sort -u > horns.tex

id_altar.tex: items sorter
	egrep -e "^139 " items | $(AWK) -F'&' -f generic-extract | sort -u > id_altar.tex

keys.tex: items sorter
	egrep -e "^21 " items | $(AWK) -F'&' -f generic-extract | sort -u > keys.tex
	egrep -e "^24 " items | $(AWK) -F'&' -f generic-extract | sort -u >> keys.tex

lighter.tex: items sorter
	egrep -e "^75 " items | $(AWK) -F'&' -f generic-extract | sort -u > lighter.tex

money.tex: items sorter
	egrep -e "^36 " items | $(AWK) -F'&' -f generic-extract | sort -u > money.tex

pit.tex: items sorter
	egrep -e "^95 " items | $(AWK) -F'&' -f generic-extract | sort -u > pit.tex

potions.tex: items sorter
	egrep -e "^5 " items | $(AWK) -F'&' -f generic-extract | sort -u > potions.tex

rings.tex: items sorter
	egrep -e "^70 " items | $(AWK) -F'&' -f generic-extract | sort -u > rings.tex

rods.tex: items sorter
	egrep -e "^3 " items | $(AWK) -F'&' -f generic-extract | sort -u > rods.tex

savebed.tex: items sorter
	egrep -e "^106 " items | $(AWK) -F'&' -f generic-extract | sort -u > savebed.tex

scrolls.tex: items sorter
	egrep -e "^111 " items | $(AWK) -F'&' -f generic-extract | sort -u > scrolls.tex

shopmat.tex: items sorter
	egrep -e "^69 " items | $(AWK) -F'&' -f generic-extract | sort -u > shopmat.tex

sign.tex: items sorter
	egrep -e "^98 " items | $(AWK) -F'&' -f generic-extract | sort -u > sign.tex

wands.tex: items sorter
	egrep -e "^109 " items | $(AWK) -F'&' -f generic-extract | sort -u > wands.tex
