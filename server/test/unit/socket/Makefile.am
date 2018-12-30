noinst_LIBRARIES = libstubs.a
libstubs_a_SOURCES=stubs_socket.c
LDADD = ../../../common/libcross.a  ../../../socket/libsocket.a libstubs.a $(LIBDL) @CHECK_LIBS@

../../logs/unit/socket:
	(mkdir ../../logs; mkdir ../../logs/unit; mkdir ../../logs/unit/socket)
../../../common/libcross.a:
	( cd ../../../common ; make libcross.a )
../../../socket/libsocket.a:
	( cd ../../../socket ; make libsocket.a )


#test unimplemented for now
#TESTS= check_image check_info check_init check_item check_loop check_lowlevel check_metaserver check_request check_sounds
TESTS=


CHECK_ROOT=../../
CHECK_FOLDER=unit/socket
CHECK_PARENT_NAME=Unit
include $(top_srcdir)/check-fragment.am

DISTCLEANFILES=	.autorun.xml

check-local: check-html
clean-local: clean-unit
