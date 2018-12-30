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
	if (/^type (.*)$/) {
	    $body_info="body_skill -1" if ($1 == 43);	# misc skill
	    $body_info="body_range -1" if ($1 == 3);	# rod
	    $body_info="body_arm -2" if ($1 == 14);	# bow
	    $body_info="body_arm -1" if ($1 == 15);	# weapon
	    $body_info="body_torso -1" if ($1 == 16);	# armor
	    $body_info="body_arm -1" if ($1 == 33);	# shield
	    $body_info="body_head -1" if ($1 == 34);	# helmet
	    $body_info="body_range -1" if ($1 == 35);	# horn
	    $body_info="body_neck -1" if ($1 == 39);	# amulet
	    $body_info="body_finger -1" if ($1 == 70);	# ring
	    $body_info="body_shoulder -1" if ($1 == 87);	# cloak
	    $body_info="body_foot -2" if ($1 == 99);	# cloak
	    $body_info="body_hand -2" if ($1 == 100);	# gloves
	    $body_info="body_wrist -2" if ($1 == 104);	# bracers
	    $body_info="body_range -1" if ($1 == 109);	# wand
	    $body_info="body_waist -1" if ($1 == 113);	# girdle
	}
	print OUT $in;
    }
    close(OUT);
    close(IN);
    unlink("$ARGV[$i].bak");
}
