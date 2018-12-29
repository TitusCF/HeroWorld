#!/usr/bin/perl

for ($i=0; $i<=$#ARGV; $i++) {
    $changed = 0;
    open(IN,"<$ARGV[$i]");
    open(OUT,">$ARGV[$i].new");
    while (<IN>) {
	$in = $_;
	if (/^can_use_armour (.*)$/) {
		print OUT "can_use_shield $1\n";
		$changed = 1;
	}
	print OUT $in;
    }
    close(OUT);
    close(IN);
    if (! $changed) {
	unlink("$ARGV[$i].new");
#	print "$ARGV[$i] unchanged\n";
    } else {
	print "$ARGV[$i] updated\n";
	unlink("$ARGV[$i]");
	if (!rename("$ARGV[$i].new", "$ARGV[$i]")) {
	    print STDERR "Unable to rename file $ARGV[$i]\n";
	}
    }
}
