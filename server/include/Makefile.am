noinst_HEADERS = \
commands.h     libproto.h     sounds.h	     attack.h       config.h \
living.h       party.h    \
player.h       define.h       dialog.h       loader.h \
race.h         material.h     spells.h       face.h         logger.h \
re-cmp.h       sproto.h       artifact.h     map.h          recipe.h \
treasure.h     global.h       path.h         shstr.h        stringbuffer.h \
tod.h          god.h          newserver.h    xdir.h         book.h \
includes.h     object.h       skills.h	    sockproto.h     plugin.h \
timers.h       typesproto.h   ob_methods.h   ob_types.h     svnversion.h \
version.h      shared/newclient.h            account_char.h \
image.h		  metaserver2.h

BUILT_SOURCES = svnversion.h

CLEANFILES = svnversion.h

svnversion.h: FORCE
	OUTPUT_DATA='/* Auto-generated at build time. */'; \
	if [ "X`echo -n`" = "X-n" ]; then \
		echo_n() { echo $${1+"$$@"}"\c"; } \
	else \
		echo_n() { echo -n $${1+"$$@"}; } \
	fi; \
	if test -n "$(SVNVERSION)" -a -d "$(top_srcdir)/.svn"; \
	then OUTPUT_DATA=`echo "$$OUTPUT_DATA"; echo_n '#define SVN_REV "'; $(SVNVERSION) -n "$(top_srcdir)"`'"'; \
	else OUTPUT_DATA=`echo "$$OUTPUT_DATA"; echo '#define SVN_REV "unknown"'`; \
	fi; \
	if test ! -e svnversion.h; then \
		echo "$$OUTPUT_DATA" > svnversion.h; \
	elif test "$$OUTPUT_DATA" != "`cat svnversion.h`"; then \
		echo "$$OUTPUT_DATA" > svnversion.h; \
	fi

FORCE:

