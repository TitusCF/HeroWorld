plugindir=$(pkglibdir)/plugins
plugin_LTLIBRARIES = cfanim.la
cfanim_la_SOURCES = cfanim.c\
	../common/plugin_common.c

cfanim_la_CPSOURCES = cfanim.c

SUBDIRS=include

cfanim_la_LIBADD =
cfanim_la_LDFLAGS = -module -avoid-version -rdynamic
AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../common/include -I$(top_srcdir)/include

proto:
	cproto -D__CEXTRACT__ -o include/cfanim_proto.h $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(cfanim_la_CPSOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 include/cfanim_proto.h
