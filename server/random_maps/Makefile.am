pkglibexec_PROGRAMS = random_map

random_map_SOURCES = main.c

noinst_LIBRARIES = librandom_map.a

librandom_map_a_SOURCES = \
	decor.c \
	door.c \
	exit.c \
	expand2x.c \
	floor.c \
	maze_gen.c \
	maze_gen.h \
	monster.c \
	random_map.c \
	random_map.h \
	reader.l \
	rogue_layout.c \
	room_gen.h \
	room_gen_onion.c \
	room_gen_spiral.c \
	snake.c \
	special.c \
	square_spiral.c \
	style.c  \
	treasure.c \
	wall.c

EXTRA_DIST = \
	README \
	expand2x.h \
	maze_gen.h \
	random_map.h \
	room_gen.h \
	rproto.h \
	main.c

AM_CPPFLAGS = -I$(top_srcdir)/include

LEX_OUTPUT_ROOT = lex.rmap
AM_LFLAGS = -Prmap -i

random_map_LDADD = ../common/libcross.a librandom_map.a

proto:
	cproto -e -o rproto.h.bak $(CPPFLAGS) -I. $(AM_CPPFLAGS) \
		`echo $(librandom_map_a_SOURCES) | $(AWK) -v RS=" " '/.c$$/ { print }'`
	sed -e "s/#if __STDC__/#ifdef __STDC__/" -e "s/__signed/signed/g" -e "/__inline/d" -e "/YY_BUFFER/d" < rproto.h.bak >rproto.h
	chmod 644 rproto.h
	rm -f rproto.h.bak

CLEANFILES = reader.c
