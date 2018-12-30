bin_PROGRAMS = crossfire-server

noinst_LIBRARIES = libserver.a
crossfire_server_SOURCES = main.c
libserver_a_SOURCES = \
	account.c\
	account_char.c\
	alchemy.c\
	apply.c\
	attack.c\
	ban.c\
	build_map.c\
	c_chat.c\
	c_misc.c\
	c_move.c\
	c_new.c\
	c_object.c\
	c_party.c\
	c_range.c\
	c_wiz.c\
	commands.c\
	daemon.c\
	disease.c\
	hiscore.c\
	gods.c\
	init.c\
	knowledge.c \
	login.c\
	monster.c\
	move.c\
    ob_methods.c\
    ob_types.c\
	party.c\
	pets.c\
	player.c\
	plugins.c\
	quest.c\
	resurrection.c\
	rune.c\
	shop.c\
	skills.c\
	skill_util.c\
	smithery.c\
	bowery.c\
	jewler.c\	
	spell_attack.c\
	spell_effect.c\
	spell_util.c\
	swap.c\
	time.c\
	timers.c\
	weather.c\
	server.c

AM_CPPFLAGS = -I$(top_srcdir)/include -DDATADIR=\"$(pkgdatadir)\" -DCONFDIR=\"$(pkgconfdir)\" \
	      -DLIBDIR=\"$(pkglibdir)\" -DLOCALDIR=\"$(pkgstatedir)\" \
	      -DPLUGIN_SUFFIX=\".so\"

if HAVE_LIBDL
crossfire_server_LDFLAGS=-export-dynamic
else
crossfire_server_LDFLAGS=
endif
crossfire_server_LDADD = libserver.a ../common/libcross.a ../random_maps/librandom_map.a \
		../socket/libsocket.a ../types/libtypes.a $(LIBS)

../common/libcross.a:
	( cd ../common ; make libcross.a )

../random_maps/librandom_map.a:
	( cd ../random_maps ; make librandom_map.a )

../socket/libsocket.a:
	( cd ../socket ; make libsocket.a )

../types/libtypes.a:
	( cd ../types ; make libtypes.a )

# Check if everything is installed as it should
installcheck-local:
	$(bindir)/crossfire-server -o

proto:
	cproto -D__CEXTRACT__ -o ../include/sproto.h $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(libserver_a_SOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 ../include/sproto.h
