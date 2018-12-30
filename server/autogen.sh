#!/bin/sh
libtoolize -f -c || exit 1
aclocal -I macros --install || exit 1
autoheader || exit 1
automake -a -c || exit 1
autoconf || exit 1
./configure $*
