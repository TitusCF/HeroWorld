#!/bin/sh

##########################
# This script will generate and insert empty test in a check file by extracting public prototype in a .c file
# The generated empty check will replace check_empty in existing unit test. Pretty usefull script to lay the path
# to testing a .c file.
#
# BACKUP file first!!!! (This script may put a mess in destination file)
# go static first!!!!! (You should first try to make everything that can go static in file static first
#
# example invocation
# utils/generateEmptyCheck.sh common/object.c test/unit/common/check_object.c
#
FILENAME=$1
OUTPUT=$2
BASENAME=`basename $FILENAME`
TEMPNAME=`tempfile`
INTERNAL_METHODS=
methods=`cproto $FILENAME -I include/ -f 1 | sed -e 's/\([^ ]\{1,\} \)\{1,\}\**\([^ ]\{1,\}\)();/\2/' -e 's/\/\*.*\*\///'`
FULLPROTOS=`tempfile`
CHECK_CODE=`tempfile`
CHECK_CALL=`tempfile`
cproto $FILENAME -I include/ -f 3 > $FULLPROTOS

echo "/*" >> $CHECK_CODE
echo " * Things to check" >> $CHECK_CODE
for i in $methods
do
    echo " * $i" >> $CHECK_CODE
done
echo " */" >> $CHECK_CODE


for i in $methods
do
PROTOTYPE=`grep -e "$i *("  $FULLPROTOS`
echo "/** This is the test to check the behaviour of the method" >> $CHECK_CODE
echo " *  $PROTOTYPE" >> $CHECK_CODE
echo " */" >> $CHECK_CODE
echo "START_TEST (test_$i)" >> $CHECK_CODE
echo "{" >> $CHECK_CODE
echo "    /*TESTME*/" >> $CHECK_CODE
echo "}" >> $CHECK_CODE
echo "END_TEST" >> $CHECK_CODE
echo >> $CHECK_CODE
echo >> $CHECK_CODE


echo "  tcase_add_test(tc_core, test_$i);" >> $CHECK_CALL
done

sed -e "/^START_TEST (test_empty)$/ {
 N
 /\n{$/ {
 N
 /\n    fail("'"'"test not yet developped"'"'");/ {
 N
 /\n{/ {
 N
 /END_TEST/ {
 N
 r $CHECK_CODE
 d
 }
 }
 }
 }
 } " -e "/^  tcase_add_test(tc_core, test_empty);/ {
 r $CHECK_CALL
 d
 }" < $OUTPUT > $TEMPNAME
cp $TEMPNAME $OUTPUT
#cat $CHECK_CODE
#cat $CHECK_CALL
rm -f $FULLPROTO
rm -f $TEMPNAME
rm -f CHECK_CODE
rm -f CHECK_CALL
