SUBDIRS = scripts Developers playbook spoiler spell-docs spoiler-html playbook-html

dist_noinst_DATA = handbook.ps spoiler.ps
dist_noinst_SCRIPTS = include_html.pl
dist_man6_MANS = crossfire-server.man crossfire-config.man \
	crossloop.web.man crossloop.man

EXTRA_DIST = README \
	alchemy.txt        commands.txt       experience.txt     knowledge.txt \
	media-tags.txt     metaserver.txt     multigod.txt       plugins.txt   \
	runes-guide.txt    settings.txt       skills.txt         stats.txt     \
	style-guide.txt    survival-guide.txt

DTOP=$(top_srcdir)/$(PACKAGE)-doc
BASENAME = @BASENAME@

spoiler.ps:
	(cd spoiler; $(MAKE) spoiler)

handbook.ps:
	(cd playbook; $(MAKE) handbook)

crosslib.doc:
	(cd ../common; make doc)

# workaround for bug in automake 1.6.2 & 1.6.3 (& probably others)
uninstall-local:
	rm -f $(DESTDIR)$(man6dir)/crossfire.6*
	rm -f $(DESTDIR)$(man6dir)/crossfire-config.6*

archive: spoiler playbook
	$(mkinstalldirs) $(DTOP)
	cp $(EXTRA_DIST) $(DTOP)
	cp spoiler.ps handbook.ps $(DTOP)
	cp spell-docs/*.txt spell-docs/*.ps $(DTOP)
	(cd spoiler-html ; make archive )
	(cd playbook-html ; make archive )
	(cd $(DTOP)/..; gtar --exclude=CVS -cvhzf $(PACKAGE)-$(VERSION).doc.tar.gz `$(BASENAME) $(DTOP)`)
	$(RM) -rf $(DTOP)
