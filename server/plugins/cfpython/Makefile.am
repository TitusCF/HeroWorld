plugindir=$(pkglibdir)/plugins

SUBDIRS = include

if PYTHON_PLUGIN
plugin_LTLIBRARIES = cfpython.la
else
plugin_LTLIBRARIES =
endif
cfpython_la_SOURCES = cfpython.c\
	cfpython_archetype.c\
	cfpython_object.c\
	cfpython_map.c\
	cfpython_party.c\
	cfpython_region.c\
	cjson.c\
	../common/plugin_common.c\
	../common/hashtable.c

cfpython_la_CPSOURCES = cfpython.c\
	cfpython_archetype.c\
	cfpython_object.c\
	cfpython_party.c\
	cfpython_region.c\
	cfpython_map.c

cfpython_la_LIBADD = @PYTHON_LIB@ @PY_LIBS@
cfpython_la_LDFLAGS = -module -avoid-version -rdynamic
AM_CPPFLAGS = -I$(srcdir)/include -I$(top_srcdir)/plugins/common/include -I$(top_srcdir)/include @PY_INCLUDES@

proto:
	cproto -D__CEXTRACT__ -o include/cfpython_proto.h $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(cfpython_la_CPSOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 include/cfpython_proto.h
