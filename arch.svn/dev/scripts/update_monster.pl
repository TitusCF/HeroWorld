#!/usr/bin/perl

for ($i=0; $i<=$#ARGV; $i++) {
    if (!rename("$ARGV[$i]", "$ARGV[$i].bak")) {
	print STDERR "Unable to rename file $ARGV[$i]\n";
	next;
    }
    print "Processing $ARGV[$i]\n";
    open(IN,"<$ARGV[$i].bak");
    open(OUT,">$ARGV[$i]");
    while (<IN>) {
	$in = $_;
	if (/^end\s*$/) {
	    print OUT "$body_info" if ($body_info ne "");
	    print OUT "$weapon_info" if ($weapon_info ne "");
	    print OUT "$wand" if ($wand ne "");
	    $body_info="";
	    $weapon_info="";
	    $wand="";
	}
	if (/^Object (.*)$/) {
	    $body_info="";
	    $weapon_info="";
	    $wand="";
	}
	elsif (/^can_use_shield\s+1/) {
	    $weapon_info="body_arm 2\n";
	}
	elsif (/^can_use_bow\s+1/) {
	    $weapon_info="body_arm 2\n";
	}
	elsif (/^can_use_weapon\s+1/) {
	    $weapon_info="body_arm 2\n";
	}
	elsif (/^can_use_wand\s+1/) {
	    $wand="body_range 1\n";
	}
	elsif (/^can_use_rod\s+1/) {
	    $wand="body_range 1\n";
	}
	elsif (/^can_use_horn\s+1/) {
	    $wand="body_range 1\n";
	}
	elsif (/^can_use_armour\s+1/) {
	    $body_info .= "body_torso 1\nbody_head 1\nbody_shoulder 1\nbody_foot 2\nbody_wrist 2\nbody_hand 2\nbody_waist 1\n";
	}
	elsif (/^can_use_ring\s+1/) {
	    $body_info .="body_finger 2\n";
	}
	elsif (/^can_use_skill\s+1/) {
	    $body_info .="body_skill 1\n";
	}

	print OUT $in;
    }
    close(OUT);
    close(IN);
    unlink("$ARGV[$i].bak");
}
