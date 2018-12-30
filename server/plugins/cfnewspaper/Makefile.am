SUBDIRS = include
plugindir=$(pkglibdir)/plugins
noinst_HEADERS =  ../common/include/plugin_common.h
if HAVE_CFNEWSPAPER
plugin_LTLIBRARIES = cfnewspaper.la
else
plugin_LTLIBRARIES =
endif

cfnewspaper_la_SOURCES = cfnewspaper.c\
	../common/plugin_common.c

cfnewspaper_la_CPSOURCES = cfnewspaper.c

cfnewspaper_la_LIBADD = -lsqlite3
cfnewspaper_la_LDFLAGS = -module -avoid-version -rdynamic
AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../common/include -I$(top_srcdir)/include

proto:
	cproto -D__CEXTRACT__ -o include/cfnewspaper_proto.h $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(cfnewspaper_la_CPSOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 include/cfnewspaper_proto.h
