noinst_LIBRARIES = libstubs.a
libstubs_a_SOURCES=stubs_random.c
LDADD = ../../../common/libcross.a libstubs.a ../../../socket/libsocket.a ../../../random_maps/librandom_map.a $(LIBDL) @CHECK_LIBS@


CHECK_ROOT=../../
CHECK_FOLDER=unit/random_maps
CHECK_PARENT_NAME=Unit
include $(top_srcdir)/check-fragment.am

../../logs/unit/random_maps:
	(mkdir ../../logs; mkdir ../../logs/unit; mkdir ../../logs/unit/random_maps)
../../../common/libcross.a:
	( cd ../../../common ; make libcross.a )
../../../socket/libsocket.a:
	( cd ../../../socket ; make libsocket.a )
../../../random_maps/librandom_map.a:
	( cd ../../../random_maps ; make librandom_map.a )

# tests non implemented, can be reactivated later on
#TESTS= check_decor check_door check_exit check_expand2x check_floor check_maze_gen check_monster check_random_map check_reader check_rogue_layout check_room_gen_onion check_room_gen_spiral check_snake check_special check_square_spiral check_standalone check_style check_test check_treasure check_wall
TESTS=

DISTCLEANFILES=	.autorun.xml

check-local: check-html
clean-local: clean-unit
