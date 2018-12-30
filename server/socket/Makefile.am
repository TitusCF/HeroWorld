noinst_LIBRARIES = libsocket.a
libsocket_a_SOURCES = \
	image.c \
	info.c \
	init.c \
	item.c\
	loop.c \
	lowlevel.c\
	metaserver.c\
	request.c\
	requestinfo.c\
	sounds.c

EXTRA_DIST = README

AM_CPPFLAGS = -I$(top_srcdir)/include

proto:
	cproto -D__CEXTRACT__ $(CPPFLAGS) $(AM_CPPFLAGS) $(libsocket_a_SOURCES) > ../include/sockproto.h
	chmod 644 ../include/sockproto.h
