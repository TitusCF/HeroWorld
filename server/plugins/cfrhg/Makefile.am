SUBDIRS = include

if CFRHG_PLUGIN
plugin_LTLIBRARIES = cfrhg.la
else
plugin_LTLIBRARIES =
endif


plugindir=$(pkglibdir)/plugins
noinst_HEADERS = include/cfrhg.h	../common/include/plugin_common.h
cfrhg_la_SOURCES = cfrhg.c\
	../common/plugin_common.c

cfrhg_la_LIBADD =
cfrhg_la_LDFLAGS = -module -avoid-version -rdynamic
AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../common/include -I$(top_srcdir)/include

proto:
	cproto -D__CEXTRACT__ -o include/cfrhg_proto.h $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(SOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 include/cfrhg_proto.h
