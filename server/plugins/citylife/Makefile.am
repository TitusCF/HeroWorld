SUBDIRS = include
plugindir=$(pkglibdir)/plugins
noinst_HEADERS = include/citylife.h	../common/include/plugin_common.h
plugin_LTLIBRARIES = citylife.la
citylife_la_SOURCES = citylife.c\
	../common/plugin_common.c

citylife_la_LIBADD =
citylife_la_LDFLAGS = -module -avoid-version -rdynamic
AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../common/include -I$(top_srcdir)/include

proto:
	cproto -D__CEXTRACT__ -o include/citylife_proto.h $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(SOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 include/citylife_proto.h
