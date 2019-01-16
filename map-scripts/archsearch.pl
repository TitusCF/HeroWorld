#!/usr/bin/perl
#
# Usage: archsearch.pl file
#
# This script searches a file (either archetypes or maps) for certain
# desired values.   It is better than grep in that it keeps track of the
# the archetype it occurs in and does smarter searches (ie, certain
# bitmask values you can search for specific bit sets, which is not
# easy to do with grep)
#
# Originally written to find all the creatures that use ghosthit.
# Modified to find all object types that are being set invisible
# in pupland
#


die("Usage: archsearch.pl file\n") if ($#ARGV < 0);

for ($file=0; $file<=$#ARGV; $file++) {
#    print "Proccessing $ARGV[$file]\n";
    if (!open(INFILE, "<$ARGV[$file]")) {
	print "Can not open $ARGV[$file] - skipping\n";
	next;
    }
    while (<INFILE>) {
	if (/^Object (\S+)/ || /^arch (\S+)/) {
		$obname = $1;
		$invisible=0;
		$type=-1;
		$x = -1;
		$y = -1;
		$title="";
	}
	$object .= $_;
	$type = $1 if (/^type (\S+)/);
	$title = $1 if (/^title (\S+)/);
	$invisible = $1 if (/^invisible (\S+)/);
	$x = $1 if (/^x (\S+)/);
	$y = $1 if (/^y (\S+)/);
	if (/^end$/ ) {
	    if ($obname =~ /^altar/ && $title ne "") {
		print "$ARGV[$file]\n$object";
	    }
#	    print "Object $obname (type $type) @ $x, $y is invisible\n";
	    $object="";
	}
	# This is what we are searching for.  value will be put in $1
	# Note that multile searches are certainly possible.
#	elsif (/^attacktype (\S+)/) {
#		if ($1 & 0x200) {
#			print "Found match, object $obname, line $.\n";
#		}
#	}

    }	# While loop
}


	
