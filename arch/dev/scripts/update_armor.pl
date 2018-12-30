#!/usr/bin/perl

for ($i=0; $i<=$#ARGV; $i++) {
    if (!rename("$ARGV[$i]", "$ARGV[$i].bak")) {
	print STDERR "Unable to rename file $ARGV[$i]\n";
	next;
    }
    open(IN,"<$ARGV[$i].bak");
    open(OUT,">$ARGV[$i]");
    while (<IN>) {
	$in = $_;
	if (/^end\s*$/) {
	    if ($body_info ne "" ) {
		print OUT "$body_info\n";
		print "Added $body_info to $ARGV[$i]\n";
	    }
	    $body_info="";
	}
	if (/^Object (.*)$/) {
	    $body_info="";
	}
	if (/^last_heal (.*)$/) {
		$body_info="gen_sp_armour $1";
		next;
	}
	print OUT $in;
    }
    close(OUT);
    close(IN);
    unlink("$ARGV[$i].bak");
}
