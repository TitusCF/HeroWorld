#!/bin/sh

#
# This will scan the whole source tree and given file in argument
# for functions that should go static or are dead
#
# example invocation:
# utils/checkInternalPrototype.sh common/object.c
#
FILENAME=$1
BASENAME=`basename $FILENAME`
TEMPNAME=`tempfile`
INTERNAL_METHODS=
methods=`cproto $FILENAME -I include/ -f 1 | sed -e 's/\([^ ]\{1,\} \)\{1,\}\**\([^ ]\{1,\}\)();/\2/' -e 's/\/\*.*\*\///'`
rm $TEMPNAME
find * -iname '*.c' -and -not -iname $BASENAME -exec cat '{}' ';' > $TEMPNAME
for i in $methods
do
MATCH=`grep -l -e "$i *(" $TEMPNAME`
if test "x$MATCH" = x; then
INTERNAL_METHODS="$INTERNAL_METHODS $i"
fi
done
for i in $INTERNAL_METHODS
do
MATCH=`grep -l -e "$i *(" $FILENAME`
if test "x$MATCH" != x; then
echo $i should be static
else
echo "$i seems a dead function (not called)"
fi
done
