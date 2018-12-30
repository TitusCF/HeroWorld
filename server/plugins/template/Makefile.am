plugindir=$(pkglibdir)/plugins
noinst_HEADERS = include/plugin_template.h	../common/include/plugin_common.h
plugin_LTLIBRARIES = plugin_template.la
plugin_template_la_SOURCES = plugin_template.c\
	../common/plugin_common.c

plugin_template_la_LIBADD =
plugin_template_la_LDFLAGS = -module -avoid-version -rdynamic
AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../common/include -I$(top_srcdir)/include

proto:
	cproto -D__CEXTRACT__ -o include/plugin_template_proto.h $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(SOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 include/plugin_template_proto.h
