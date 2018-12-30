plugindir=$(pkglibdir)/plugins
noinst_HEADERS = include/cflogger.h	include/cflogger_proto.h ../common/include/plugin_common.h
if HAVE_CFLOGGER
plugin_LTLIBRARIES = cflogger.la
else
plugin_LTLIBRARIES =
endif

cflogger_la_SOURCES = cflogger.c\
	../common/plugin_common.c

cflogger_la_CPSOURCES = cflogger.c

cflogger_la_LIBADD = -lsqlite3
cflogger_la_LDFLAGS = -module -avoid-version -rdynamic
AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../common/include -I$(top_srcdir)/include

proto:
	cproto -D__CEXTRACT__ -o include/cflogger_proto.h $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(cflogger_la_CPSOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 include/cflogger_proto.h
