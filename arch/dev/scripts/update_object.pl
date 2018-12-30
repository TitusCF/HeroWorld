#!/usr/bin/perl
$TYPE = $ARGV[0];
die ("type is 0: $TYPE\n") if (($TYPE + 1) == 1);

for ($i=1; $i<=$#ARGV; $i++) {
    if (!rename("$ARGV[$i]", "$ARGV[$i].bak")) {
	print STDERR "Unable to rename file $ARGV[$i]\n";
	next;
    }
    open(IN,"<$ARGV[$i].bak");
    open(OUT,">$ARGV[$i]");
    while (<IN>) {
	$in = $_;
	if (/^end\s*$/) {
	    print OUT "name_pl $namepl\n";
	    print OUT "client_type $TYPE\n";
	    $namepl="";
	}
	if (/^Object (.*)$/) { 
	    $namepl = $1."s";
	}
	if (/^name (.*)$/) { 
	    $namepl = $1."s";
	}
	print OUT $in;
    }
    close(OUT);
    close(IN);
    unlink("$ARGV[$i].bak");
}
