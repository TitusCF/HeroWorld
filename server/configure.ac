# Process this file with autoconf to produce a configure script.

AC_INIT([crossfire], [2.1.1], [crossfire@metalforge.org])
AC_CONFIG_AUX_DIR([utils])
AC_CONFIG_HEADERS([include/autoconf.h])
AC_CONFIG_MACRO_DIR([macros])
AC_CONFIG_SRCDIR([server/main.c])
AC_PREFIX_DEFAULT([/usr/games/crossfire])
AM_INIT_AUTOMAKE


##############################################################################
# Configuration Options
##############################################################################
AC_ARG_ENABLE(mapper, [AS_HELP_STRING([--enable-mapper],
              [Build the crossfire-mapper utility (requires libgd)])],
              [enable_mapper=yes], [enable_mapper=no])

# Options for plugins
AC_ARG_WITH(python, [AS_HELP_STRING([--with-python=dir],
            [Directory to look for python headers/library])],
            [PYTHON_HOME="$withval"])

AC_ARG_ENABLE(cfrhg, [AS_HELP_STRING([--enable-cfrhg],
              [Build the random house generator plugin])],
              [], [enable_cfrhg=no])

AC_ARG_ENABLE(logger, [AS_HELP_STRING([--enable-logger],
              [Build the SQLite logging plugin])],
              [enable_logger=yes], [enable_logger=no])

AC_ARG_ENABLE(newspaper, [AS_HELP_STRING([--enable-newspaper],
              [Build the newspaper plugin])],
              [enable_newspaper=yes], [enable_newspaper=no])

# Unit test configuration
AC_ARG_ENABLE(check, [AS_HELP_STRING([--disable-check],
              [Disable the unit tests even if check is available])],
              [enable_check=no], [enable_check=yes])

# Add support for dmalloc, if the user wants it.
AM_WITH_DMALLOC

##############################################################################
# Program Checks
##############################################################################
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LIBTOOL
AM_PROG_LEX

AC_PATH_PROG(BASENAME, basename)
AC_PATH_PROG(BZIP, bzip2)
AC_PATH_PROG(COMPRESS, compress)
AC_PATH_PROG(GZIP, gzip)
AC_PATH_PROG(LATEX, latex)
AC_PATH_PROG(PERL, perl)
AC_PATH_PROG(SVNVERSION, svnversion)
AC_PATH_PROG(TAR, tar)

AC_SUBST(BASENAME)
AC_SUBST(PERL)

if test -z "$COMPRESS" -a -z "$GZIP" -a -z "$BZIP" ; then
    AC_MSG_NOTICE([Unable to find compress, gzip, or bzip2])
fi;

AC_DEFINE_UNQUOTED(BZIP,"${BZIP}",[Path to the bzip binary])
AC_DEFINE_UNQUOTED(COMPRESS,"${COMPRESS}",[Path to the compress binary])
AC_DEFINE_UNQUOTED(GZIP,"${GZIP}",[Path to the gzip binary])

##############################################################################
# Library Checks
##############################################################################
# Check for some library functions
AC_SEARCH_LIBS([gethostbyname], [nsl])
AC_SEARCH_LIBS([pow], [m])
AC_SEARCH_LIBS([sqrt], [m])

# Misc libraries.
AC_SEARCH_LIBS([crypt], [crypt])
AC_CHECK_LIB(des, des_crypt)

AC_CHECK_LIB(z, compress)
AC_CHECK_LIB(pthread, pthread_create)

LIBCURL_CHECK_CONFIG()
LIBS="$LIBS $LIBCURL"
CPPFLAGS="$CPPFLAGS $LIBCURL_CPPFLAGS"

# For utils/mapper
if test "$enable_mapper" = "yes"; then
    PKG_CHECK_MODULES([GD], [gdlib])
    AC_SUBST(GD_LIBS, $GD_LIBS)
fi
AM_CONDITIONAL(MAPPER, test "$enable_mapper" = "yes")

# Random house generator
AM_CONDITIONAL(CFRHG_PLUGIN, test "$enable_cfrhg" = "yes")

# Libraries used for plugins
AC_SEARCH_LIBS(dlopen, [dl], [ cf_have_libdl=yes ])
AC_CHECK_LIB(sqlite3, sqlite3_open,[ cf_have_libsqlite=yes ])

##############################################################################
# Header File, Type, Structure, and Compiler Checks
##############################################################################
# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h stddef.h stdlib.h strings.h sys/file.h sys/ioctl.h sys/time.h time.h unistd.h sys/ttycom.h termios.h sys/termios.h crypt.h des.h zlib.h pthread.h curl/curl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_HEADER_STDBOOL
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)

CF_CHECK_VISIBILITY()

##############################################################################
# Library Function Checks
##############################################################################
# Checks for library functions.
AC_FUNC_MEMCMP
AC_FUNC_SETPGRP
AC_FUNC_STRERROR_R
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF

AC_CHECK_FUNCS([getdtablesize gettimeofday mkdir mktime rmdir select socket strcasecmp strcspn strerror strncasecmp strspn strstr strtol snprintf setsid srandom srand48 srand sysconf scandir getaddrinfo getnameinfo])

# alphasort can not be added to AC_CHECK_FUNCS because
AC_MSG_CHECKING(checking if alphasort is defined)
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <dirent.h>]], [[alphasort;]])],[ac_cv_func_alphasort=yes],[ac_cv_func_alphasort=no])
  if test "$ac_cv_func_alphasort" = yes; then
    AC_DEFINE(HAVE_ALPHASORT, 1, [Define if you have the alphasort() function declaration.])
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi

AC_MSG_CHECKING(how many args gettimeofday uses)
AC_CACHE_VAL(ac_cv_gettimeofday_args,
 [AC_TRY_COMPILE([#include <sys/time.h>
	#include <unistd.h>],
	[struct timeval tv; struct timezone tzp;
	gettimeofday(&tv, &tzp);],
	[ac_gettimeofday_args=2],
		[AC_TRY_COMPILE([#include <sys/time.h>
		#include <unistd.h>],
		[struct timeval tv;
		gettimeofday(&tv, NULL);],
		[ac_gettimeofday_args=2],
		[AC_TRY_COMPILE([#include <sys/time.h>
			#include <unistd.h>],
			[struct timeval tv; gettimeofday(&tv);],
			[ac_gettimeofday_args=1],
			[ac_gettimeofday_args=0])])])
  ac_cv_gettimeofday_args=$ac_gettimeofday_args])

ac_gettimeofday_args=$ac_cv_gettimeofday_args
if test $ac_gettimeofday_args = 1 ; then
  AC_DEFINE(HAVE_GETTIMEOFDAY,[],[Define if gettimeofday is available])
  AC_MSG_RESULT(one argument)
elif test $ac_gettimeofday_args = 2 ; then
  AC_DEFINE(HAVE_GETTIMEOFDAY,[],[Define if gettimeofday is available])
  AC_DEFINE(GETTIMEOFDAY_TWO_ARGS,[],[Define if gettimeofday takes two arguments])
  AC_MSG_RESULT(two arguments)
else
  AC_MSG_RESULT(unknown)
fi

AC_SUBST(pkgstatedir,$localstatedir/$PACKAGE)
AC_SUBST(pkgconfdir,$sysconfdir/$PACKAGE)

#############################################
# Plugin configuration
AM_CONDITIONAL(HAVE_LIBDL, test "$cf_have_libdl" = "yes")

# Test for libdl isn't taken into account for many plugins, don't take into
# account for Python. Fixes bug #3353920.
# Besides, one can hope modern systems do have that function.

#if test "$cf_have_libdl" = "yes" ; then
    ###############
    # Python plugin
    CF_CHECK_PYTHON(
        [PLUGIN_PYTHON="cfpython.la"]
        [cf_will_build_python="yes"],
        [cf_will_build_python="no"]
        [AC_MSG_NOTICE([No Python found. Python plugin will not be built.])]
    )
    # *** Put other plugins configuration code here ***
#else
#    AC_MSG_NOTICE([No dl library found. Plugins will not be supported.])
#fi
AM_CONDITIONAL(PYTHON_PLUGIN,test "x$PLUGIN_PYTHON" != "x")
AC_SUBST(PLUGIN_PYTHON)

AM_CONDITIONAL(HAVE_CFLOGGER, [test "$cf_have_libsqlite" = "yes" -a "$enable_logger" = "yes"])
AM_CONDITIONAL(HAVE_CFNEWSPAPER, [test "$cf_have_libsqlite" = "yes" -a "$enable_newspaper" = "yes"])

# Unit tests
AM_PATH_CHECK([], [have_check="yes"], [have_check="no"])
AM_CONDITIONAL(HAVE_CHECK, test "$have_check" = "yes" -a "$enable_check" = "yes")

if test "$have_check" = "yes"; then
    CF_CHECK_XSLT(xsltengine)
    AC_SUBST(XSLTENGINE, ["$xsltengine"])
else
    AC_MSG_WARN([Check not found; cannot run unit tests!])
fi

AM_CONDITIONAL(PROCESS_XSLT, [test -n "$xsltengine"])

##############################################################################
# Generate Output
##############################################################################
AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 doc/Developers/Makefile
                 doc/playbook/Makefile doc/playbook-html/Makefile
                 doc/scripts/Makefile
                 doc/spell-docs/Makefile
                 doc/spoiler/Makefile doc/spoiler-html/Makefile
                 common/Makefile
                 include/Makefile
                 lib/Makefile
                 random_maps/Makefile
                 server/Makefile
                 socket/Makefile
                 types/Makefile
                 utils/Makefile
                 lib/checkarch.pl lib/collect.pl
                 utils/add_throw.perl utils/metaserver.pl utils/scores.pl utils/player_dl.pl
                 utils/crossloop.web
                 plugins/Makefile
                 plugins/cfanim/Makefile plugins/cfanim/include/Makefile
                 plugins/cflogger/Makefile
                 plugins/cfnewspaper/Makefile
                 plugins/cfnewspaper/include/Makefile
                 plugins/cfpython/Makefile plugins/cfpython/include/Makefile
                 plugins/cfrhg/Makefile
                 plugins/cfrhg/include/Makefile
                 plugins/citylife/Makefile
                 plugins/citylife/include/Makefile
                 plugins/common/Makefile plugins/common/include/Makefile
                 test/Makefile
                 test/bugs/Makefile test/bugs/bugtrack/Makefile
                 test/include/Makefile test/templates/Makefile test/toolkit/Makefile
                 test/unit/Makefile
                 test/unit/common/Makefile test/unit/random_maps/Makefile
                 test/unit/server/Makefile test/unit/socket/Makefile])
AC_OUTPUT

##############################################################################
# Configuration Summary
##############################################################################
AC_MSG_NOTICE([Configuration Summary ----------------------------------------------])
AC_MSG_NOTICE([])
AC_MSG_NOTICE([  Paths])
AC_MSG_NOTICE([    Installation prefix                  ${prefix}])
AC_MSG_NOTICE([    Will put executables in              $bindir])
AC_MSG_NOTICE([    Will put resources in                $datadir])
AC_MSG_NOTICE([    Will put config in                   $sysconfdir])
AC_MSG_NOTICE([    Will put server runtime data in      $localstatedir])
AC_MSG_NOTICE([])
AC_MSG_NOTICE([  Options])
AC_MSG_NOTICE([    Build metaserver2 support?      $ac_cv_header_curl_curl_h])
AC_MSG_NOTICE([    Build plugins?                  $cf_have_libdl])

if test "$cf_have_libsqlite" = "yes" ; then
AC_MSG_NOTICE([    Build cflogger?                 $enable_logger])
AC_MSG_NOTICE([    Build cfnewspaper?              $enable_newspaper])
fi

if test "$cf_have_libdl" = "yes" ; then
AC_MSG_NOTICE([    Build python plugin?            $cf_will_build_python])
fi

if test "$enable_cfrhg" = "yes" ; then
AC_MSG_NOTICE([    Build random house plugin?      yes])
else
AC_MSG_NOTICE([    Build random house plugin?      no])
fi

if test x"$xsltengine" != "x" ; then
    check_complement=["(will generate report)"]
fi
if test "$have_check" = "yes" -a "$enable_check" = "yes" ; then
AC_MSG_NOTICE([    Process unit testing?           yes $check_complement])
AC_MSG_NOTICE([      (run make; make -k check)])
else
AC_MSG_NOTICE([    Process unit testing?           no])
AC_MSG_NOTICE([      If you are a developer, you MUST install the check framework])
AC_MSG_NOTICE([      and write automated tests for all your add-ons!])
fi
AC_MSG_NOTICE([])
AC_MSG_NOTICE([  Utilities (not required to build or run the server)])
AC_MSG_NOTICE([    Build crossfire-mapper?         $enable_mapper])
AC_MSG_NOTICE([--------------------------------------------------------------------])
AC_MSG_NOTICE([Remember to add a symlink to your 'arch' directory in 'lib/arch'!])
