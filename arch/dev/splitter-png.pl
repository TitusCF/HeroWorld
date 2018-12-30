#!/usr/bin/perl

# File by mwedel@scruz.net to deal with splitting png's.
# Takes any number of files on the command line, but all the split images
# are dumped in the current directory.
# does preserve alpha channel.  Tested on skree images.

$PNGSIZE=32;

for ($i=0; $i <= $#ARGV; $i++) {
    @name = split(m#/#, $ARGV[$i]);
    $source = $name[$#name];
    $#name--;
    $destdir=join(m#/#, @name);
    
    if ($source =~ /(\w+)\.(\d)(\d)(\d)\.png/ ) {
	$base = $1;
	$facing = $3;
	$animation = $4;
    } else {
	print STDERR "filename $source does not follow proper naming conventions - don't know how to name files - skipping\n";
	next;
    }

    if (!open(DESC,"pngtopnm $ARGV[$i] | pnmfile |")) {
	print STDERR "Could not run pngtopnm $source | pnmfile\n";
	next;
    }
    $desc = <DESC>;
    close(DESC);
    $desc =~ /(\d+) by (\d+)/;
    if ($1 == 0 || $2 == 0) {
	print STDERR "File $ARGV[$i] - file not described properly\n";
	next;
    }
    if (($1 % 32) || ($2 % 32)) {
	print STDERR "File $ARGV[$i] not an increment of 32x32 size - size is $1x$2\n";
	next;
    }
    $destx = $1/32;
    $desty = $2/32;
    print "File $ARGV[$i] will be split into $destx X $desty pieces\n";

    $piece = 0;
    # The order of the for loops of x and y seem non intuitive, but believe
    # me, this does left to right, top to bottom (ie, as you read and
    # english language book) in terms of part numbering.

    for ($y=0; $y < $desty; $y++) {
	for ($x=0; $x < $destx; $x++) {
	    $piece ++;
	    # Deal with naming convention.  piece 10 should be A, A ascii
	    # code starts at 65.
	    if ($piece > 9 ) {
		$np = chr($piece + 55);
	    } else {
		$np = $piece;
	    }
	    $cutx = $x * 32;
	    $cuty = $y * 32;
	    system("pngtopnm -alpha $ARGV[$i] | pnmcut -left $cutx -top $cuty -width 32 -height 32 > /tmp/alpha.$$");
	    system("pngtopnm $ARGV[$i] | pnmcut -left $cutx -top $cuty -width 32 -height 32 > /tmp/png.$$");
	    system("pnmtopng -alpha /tmp/alpha.$$ /tmp/png.$$ > $base.$np$facing$animation.png");
	}
    }
    # do a little cleanup 
    unlink("/tmp/alpha.$$");
    unlink("/tmp/png.$$");
}
