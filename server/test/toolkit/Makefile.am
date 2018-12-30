AM_CPPFLAGS = -I$(top_srcdir)/include -I../include -DRESSOURCES=\"$(srcdir)/../../resources\" -DLOGDIR=\"$(top_builddir)/test/logs\" -DSOURCE_ROOT=\"$(srcdir)/../../\"
noinst_LIBRARIES = libtoolkit_common.a libtoolkit_random_maps.a libtoolkit_socket.a libtoolkit_server.a libtoolkit_crossedit.a

libtoolkit_common_a_SOURCES = toolkit_common.c
libtoolkit_random_maps_a_SOURCES = toolkit_random_maps.c
libtoolkit_socket_a_SOURCES = toolkit_socket.c
libtoolkit_server_a_SOURCES = toolkit_server.c
libtoolkit_crossedit_a_SOURCES = toolkit_crossedit.c

proto:
	cproto -D__CEXTRACT__ -o ../include/toolkit_common.h $(INCLUDES) $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(libtoolkit_common_a_SOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 ../include/toolkit_common.h
	cproto -D__CEXTRACT__ -o ../include/toolkit_random_maps.h $(INCLUDES) $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(libtoolkit_random_maps_a_SOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 ../include/toolkit_random_maps.h
	cproto -D__CEXTRACT__ -o ../include/toolkit_socket.h $(INCLUDES) $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(libtoolkit_socket_a_SOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 ../include/toolkit_socket.h
	cproto -D__CEXTRACT__ -o ../include/toolkit_server.h $(INCLUDES) $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(libtoolkit_server_a_SOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 ../include/toolkit_server.h
	cproto -D__CEXTRACT__ -o ../include/toolkit_crossedit.h $(INCLUDES) $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(libtoolkit_crossedit_a_SOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 ../include/toolkit_crossedit.h
