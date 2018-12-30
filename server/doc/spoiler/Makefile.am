EXTRA_DIST = monster-extract spells-extract stats-extract arm-extract\
 	helm-extract mag-extract shield-extract items-extract spoiler.tex\
 	sorter empty.pbm bow-extract arche-extract weap-extract

TEX =	monput.tex spells.tex stats.tex weap.tex shield.tex helmet.tex\
 	mag.tex arm.tex arche.tex bow.tex version.tex weapmag.tex

CLEANFILES =  $(TEX) items .bmaps in_monput in_items in_spells in_stats \
	*.ps *x*.ppm tmp.ppm work.ppm spoiler.dvi spoiler.log spoiler.aux

CROSSBIN = ../../server/crossfire-server
INCDIR = $(top_srcdir)/include
PERL = @PERL@

spoiler: ../spoiler.ps

../spoiler.ps: spoiler.dvi
	dvips -o ../spoiler.ps spoiler.dvi

spoiler.dvi: spoiler.aux
	latex spoiler.tex

spoiler.aux: spoiler.tex $(TEX)
	$(RM) spoiler.aux
	latex spoiler.tex

dump_switches:
	@$(RM) dump_switches
	@$(CROSSBIN) -m >/dev/null 2>&1 && touch dump_switches ||	(echo "You need to define DUMP_SWITCHES in include/config.h and";	echo "recompile before you can create the spoilers.")

	@$(MAKE) $(MFLAGS) check_ok

check_ok: dump_switches
	@$(RM) dump_switches

.bmaps: ../../lib/archetypes
	$(RM) *.ps
	touch .bmaps

items: .bmaps in_items ../scripts/makeps.pl items.ps
	$(PERL) ../scripts/makeps.pl output=tex libdir=$(top_srcdir)/lib archdir=$(top_srcdir)/lib/ size=0.4 input=in_items inarch=$(pkgdatadir)/archetypes > items

in_items: items-extract $(pkgdatadir)/archetypes $(top_srcdir)/common/living.c
	$(AWK) -v living_c=$(top_srcdir)/common/living.c 		-f items-extract $(pkgdatadir)/archetypes > in_items

stats.tex: .bmaps in_stats ../scripts/makeps.pl characters.ps
	$(PERL) ../scripts/makeps.pl output=tex libdir=$(top_srcdir)/lib archdir=$(top_srcdir)/lib/ size=0.4 input=in_stats inarch=$(pkgdatadir)/archetypes > stats.tex

in_stats: stats-extract $(pkgdatadir)/archetypes $(top_srcdir)/common/living.c
	$(AWK) -v living_c=$(top_srcdir)/common/living.c 		-f stats-extract $(pkgdatadir)/archetypes > in_stats

monput.tex: .bmaps in_monput ../scripts/makeps.pl monsters.ps
	$(PERL) ../scripts/makeps.pl output=tex libdir=$(top_srcdir)/lib archdir=$(top_srcdir)/lib/ size=0.4 input=in_monput inarch=$(pkgdatadir)/archetypes > monput.tex

in_monput: monster-extract $(CROSSBIN) $(pkgdatadir)/treasures $(pkgdatadir)/archetypes
	$(CROSSBIN) -m2 | sort -f | 		$(AWK) -F'|' -f monster-extract > in_monput

spells.tex: .bmaps in_spells ../scripts/makeps.pl spells.ps
	$(PERL) ../scripts/makeps.pl output=tex libdir=$(top_srcdir)/lib archdir=$(top_srcdir)/lib/ size=0.4 input=in_spells inarch=$(pkgdatadir)/archetypes > spells.tex

in_spells: spells-extract sorter $(INCDIR)/spellist.h $(CROSSBIN) $(pkgdatadir)/archetypes
	tr -d \" < $(INCDIR)/spellist.h | 		$(AWK) -F, -v crosscmd="$(CROSSBIN) -m4" 			-f spells-extract | sort > in_spells

version.tex: $(CROSSBIN)
	$(CROSSBIN) -v 2>&1 | grep "^This is" | 		cut -d" " -f3- > version.tex

characters.ps items.ps monsters.ps spells.ps:
	touch $@

weap.tex: items sorter
	egrep -e "^15 " items | $(AWK) -F'&' '{printf("%s &%s &%s &%.2f &%s\n",$$3,$$10,$$5,$$11,$$6)}' |	sort | $(AWK) -F'&' -f sorter -v lengde=`egrep -e "^15" items | 	wc -l | tr -d ' ' ` | sort | cut -d'&' -f 2- > weap.tex

shield.tex: items shield-extract
	egrep -e "^33 " items | $(AWK) -F'&' -f shield-extract | sort > shield.tex

arm.tex: items arm-extract
	egrep -e "^16 " items | $(AWK) -F'&' -f arm-extract | sort -t'&' -n +4 -5 > arm.tex

helmet.tex: items helm-extract
	egrep -e "^34 " items | $(AWK) -F'&' -f helm-extract | 		sort -t'&' -n +3 -4 | sed -e s/_/' '/g > helmet.tex

bow.tex: items bow-extract
	egrep -e "^14 " items | $(AWK) -F'&' -f bow-extract | sort > bow.tex

arche.tex: items arche-extract
	egrep -e "^915 " items | $(AWK) -F'&' -f arche-extract | sort > arche.tex

weapmag.tex: items weap-extract
	egrep -e "^15 " items | $(AWK) -F'&' -f weap-extract | sort > weapmag.tex

mag.tex: items mag-extract
	$(AWK) -F'&' -f mag-extract items | sort +0.9 -0.10 > mag.tex
