EXTRA_DIST = spells-extract stats-extract\
	spoiler.shtml helm-extract mag-extract shield-extract \
	empty.pbm bow-extract arche-extract weap-extract

HTML =	monput.html spells.html stats.html weap.html shield.html helmet.html\
 	mag.html arm.html arche.html bow.html version.html weapmag.html

CLEANFILES = items .bmaps in_monput in_items in_spells in_stats \
		*x*.ppm tmp.ppm work.ppm *.png $(HTML) spoiler.html

CROSSBIN = ../../server/crossfire-server
INCDIR = $(top_srcdir)/include
DTOP = $(top_srcdir)/$(PACKAGE)-doc
PERL = @PERL@

spoiler.html: $(HTML) spoiler.shtml
	$(PERL) $(srcdir)/../include_html.pl spoiler.shtml spoiler.html

dump_switches: $(CROSSBIN)
	@$(RM) dump_switches
	@$(CROSSBIN) -m >/dev/null 2>&1 && touch dump_switches ||	(echo "You need to define DUMP_SWITCHES in include/config.h and";	echo "recompile before you can create the spoilers.")
	@$(MAKE) $(MFLAGS) check_ok

$(CROSSBIN):
	( cd $(top_srcdir)/server ; make crossfire )

check_ok: dump_switches
	@$(RM) dump_switches

.bmaps: $(pkgdatadir)/archetypes
	$(RM) *.gif
	touch .bmaps

items: .bmaps in_items ../scripts/makeps.pl items.gif
	$(PERL) ../scripts/makeps.pl output=png libdir=$(top_srcdir)/lib archdir=$(top_srcdir)/lib/ size=0.4 input=in_items inarch=$(pkgdatadir)/archetypes > items

in_items: $(srcdir)/../scripts/items-extract.pl $(pkgdatadir)/archetypes $(top_srcdir)/common/living.c
	$(PERL) $(srcdir)/../scripts/items-extract.pl  living_c=$(top_srcdir)/common/living.c $(pkgdatadir)/archetypes > in_items

stats.html: .bmaps in_stats ../scripts/makeps.pl stats.gif
	$(PERL) ../scripts/makeps.pl output=png libdir=$(top_srcdir)/lib archdir=$(top_srcdir)/lib/ size=0.4 input=in_stats inarch=$(pkgdatadir)/archetypes > stats.html

in_stats: stats-extract $(pkgdatadir)/archetypes $(top_srcdir)/common/living.c
	$(AWK) -v living_c=$(top_srcdir)/common/living.c 		-f stats-extract $(pkgdatadir)/archetypes > in_stats

monput.html: .bmaps in_monput ../scripts/makeps.pl monsters.gif
	$(PERL) ../scripts/makeps.pl output=png libdir=$(top_srcdir)/lib archdir=$(top_srcdir)/lib/ size=0.4 input=in_monput inarch=$(pkgdatadir)/archetypes > monput.html

in_monput: ../scripts/monster-extract.pl $(CROSSBIN) $(pkgdatadir)/treasures $(pkgdatadir)/archetypes
	$(CROSSBIN) -m2 |  sort -f | $(PERL) ../scripts/monster-extract.pl > in_monput

spells.html: .bmaps in_spells ../scripts/makeps.pl spells.gif
	$(PERL) ../scripts/makeps.pl output=png libdir=$(top_srcdir)/lib archdir=$(top_srcdir)/lib/ size=0.4 input=in_spells inarch=$(pkgdatadir)/archetypes > spells.html

in_spells: spells-extract  $(INCDIR)/spellist.h 		$(CROSSBIN) $(pkgdatadir)/archetypes
	tr -d \" < $(INCDIR)/spellist.h | 		$(AWK) -F, -v crosscmd="$(CROSSBIN) -m4" 			-f spells-extract | sort > in_spells

version.html: $(CROSSBIN)
	$(CROSSBIN) -v 2>&1 | grep "^This is" | 		cut -d" " -f3- > version.html

stats.gif items.gif monsters.gif spells.gif:
	touch $@

weap.html: items
	egrep -e "^15 " items | $(AWK) -F'&' '{printf("<tr><th>%s</th><td>%s</td><td>%s</td><td>%.2f</td><td>%s</td></tr>\n",$$3,$$10,$$5,$$11,$$6)}' > weap.html

shield.html: items shield-extract
	egrep -e "^33 " items | $(AWK) -F'&' -f shield-extract | sort > shield.html

arm.html: items arm-extract
	egrep -e "^16 " items | $(AWK) -F'&' -f arm-extract | sort -t'&' -n +4 -5 > arm.html

helmet.html: items helm-extract
	egrep -e "^34 " items | $(AWK) -F'&' -f helm-extract | 		sort -t'&' -n +3 -4 | sed -e s/_/' '/g > helmet.html

bow.html: items bow-extract
	egrep -e "^14 " items | $(AWK) -F'&' -f bow-extract | sort > bow.html

arche.html: items arche-extract
	egrep -e "^915 " items | $(AWK) -F'&' -f arche-extract | sort > arche.html

weapmag.html: items weap-extract
	egrep -e "^15 " items | $(AWK) -F'&' -f weap-extract | sort > weapmag.html

mag.html: items mag-extract
	$(AWK) -F'&' -f mag-extract items | sort +0.9 -0.10 > mag.html

archive: spoiler.html
	$(mkinstalldirs) $(DTOP)/spoiler-html
	cp spoiler.html $(DTOP)/spoiler-html
	cp *.gif $(DTOP)/spoiler-html
