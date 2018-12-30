#!/usr/bin/perl
# Adds the throwing skill to to the old player files.  This script
# is likely no longer needed, since skills have been around for quite a while.
#

$end = 0;
if (!-e $ARGV[0]) {
	die("Can not find file $ARGV[0]\n");
}
if (-e $ARGV[0].".old") {
	die("$ARGV[0].new already exists.\n");
}
rename($ARGV[0], $ARGV[0].".old");
open(PLAYER,"<".$ARGV[0].".old") || die("can not open $ARGV[0].old");
open(PL_NEW,">".$ARGV[0]) || die("can not open $ARGV[0]");

while (<PLAYER>) {
	if (/^end$/) {
		if ($end) {
			print PL_NEW "arch skill_throwing\nend\nend\n";
		}
		else {print PL_NEW $_;}
		$end=1;
	}
	else {
		print PL_NEW $_;
		$end=0;
	}
}
close(PLAYER);
close(PL_NEW);
