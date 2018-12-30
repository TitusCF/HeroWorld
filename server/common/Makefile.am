noinst_LIBRARIES = libcross.a

libcross_a_SOURCES = \
	anim.c \
	arch.c\
  artifact.c\
	button.c\
	dialog.c \
	exp.c\
	friend.c\
	glue.c\
	holy.c\
	info.c\
	image.c \
	init.c\
	item.c\
	languages.c\
	links.c\
	living.c\
	loader.l\
	logger.c\
	los.c\
	map.c\
	ob_methods.c \
	ob_types.c \
	object.c\
	path.c\
	porting.c\
	player.c\
	re-cmp.c\
	readable.c\
	recipe.c\
	region.c\
	shstr.c\
	stringbuffer.c\
	time.c\
	treasure.c\
	utils.c

EXTRA_DIST = README
AM_CPPFLAGS = -I$(top_srcdir)/include -DDATADIR=\"$(pkgdatadir)\" -DCONFDIR=\"$(pkgconfdir)\" \
	      -DLIBDIR=\"$(pkglibdir)\" -DLOCALDIR=\"$(pkgstatedir)\"
AM_LFLAGS = -i

proto:
	cproto -e -o ../include/libproto.h.bak $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(SOURCES) loader.c " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	sed -e "s/__signed/signed/g" -e "/inline/d" -e "/YY_BUFFER/d" -e "/yy_size_t/d" < ../include/libproto.h.bak > ../include/libproto.h
	chmod 644 ../include/libproto.h
	rm -f ../include/libproto.h.bak

CLEANFILES = loader.c
