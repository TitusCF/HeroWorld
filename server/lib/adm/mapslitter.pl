#!/usr/bin/perl

# This script will write (to stdout) all the needed exits to connect maps
# in a tiled fashion.  The variables at the start will need to be set
# for things to work.

# Set these as appropriate to the maps it should connect to.  If one is left
# blank, then exits for that direction will not be created.

$MAPNAME=$ARGV[0];
$XM=$ARGV[1];
$YM=$ARGV[2];
$WIDTH=$ARGV[3];
$HEIGHT=$ARGV[4];
#$SPLITMAP=$ARGV[5];

if ($MAPNAME eq "") {
    die "Usage: connect.pl <basename> <x maps> <y maps> <width> <height>";
}
if ($WIDTH==0) {
    $WIDTH=42;
}
if ($HEIGHT==0) {
    $HEIGHT=34;
}

# DELTA What the overlap is - it should always be 5 for smooth transitions
$DELTA=5;
$xc=1;
$yc=1;

print STDOUT "Creating connection maps.\n";

while ($xc<=$XM) {
    $yc=1;
    while ($yc<=$YM) {
	$NORTH="";
	$NORTHWEST="";
	$WEST="";
	$SOUTHWEST="";
	$SOUTH="";
	$SOUTHEAST="";
	$EAST="";
	$NORTHEAST="";
	if ($yc>1){
	    $NORTH=$MAPNAME."_".$xc."_".($yc-1);
	}
	if ($yc>1 || $xc>1) {
	    $NORTHWEST=$MAPNAME."_".($xc-1)."_".($yc-1);
	}
	if ($xc>1) {
	    $WEST=$MAPNAME."_".($xc-1)."_".$yc;
	}
	if ($xc>1 || $yc<$YM) {
	    $SOUTHWEST=$MAPNAME."_".($xc-1)."_".($yc+1);
	}
	if ($yc<$YM) {
	    $SOUTH=$MAPNAME."_".$xc."_".($yc+1);
	}
	if ($yc<$YM || $xc<$XM) {
	    $SOUTHEAST=$MAPNAME."_".($xc+1)."_".($yc+1);
	}
	if ($xc<$XM) {
	    $EAST=$MAPNAME."_".($xc+1)."_".$yc;
	}
	if ($xc<$XM || $yc>1) {
	    $NORTHEAST=$MAPNAME."_".($xc+1)."_".($yc-1);
	}
	$THISMAP=$MAPNAME."_".$xc."_".$yc;
	open (MAP, ">$THISMAP") or die "unable to open mapfile.";
	print MAP "arch map\n";
	print MAP "name $MAPNAME\n";
	print MAP "msg\n";
	print MAP "Creator: splitmap.pl\n";
	print MAP "Email: azzie\@tellutec.se\n";
	print MAP "Date: Wed Oct 27 10:59:23 1993\n";
	print MAP "endmsg\n";
	print MAP "hp ".($DELTA+1)."\n";
	print MAP "sp $DELTA\n";
	print MAP "x $WIDTH\n";
	print MAP "y $HEIGHT\n";
	print MAP "end\n";
	print $MAPNAME."_".$xc."_".$yc."\n";
#	print "XC=".$xc."\n";
#	print "YC=".$yc."\n";
#$NORTHWEST="";
#$WEST="";
#$SOUTHWEST="";
#$SOUTH="world_a3";
#$SOUTHEAST="world_b3";
#$EAST="world_b2";
#$NORTHEAST="world_b1";
# End of configurable options.
# Quick reminder - hp is the destination x, sp is the destination y

# Lets do the corners first
	if ($NORTHWEST ne "") {
	    print MAP "arch exit\n";
	    print MAP "slaying $NORTHWEST\n";
	    print MAP "x ".($DELTA-1)."\n";
	    print MAP "y ".($DELTA-1)."\n";
	    print MAP "hp ".($WIDTH-$DELTA-1)."\n";
	    print MAP "sp ".($HEIGHT-$DELTA-1)."\n";
	    print MAP "end\n";
	}
	if ($SOUTHWEST ne "") {
	    print MAP "arch exit\n";
	    print MAP "slaying $SOUTHWEST\n";
	    print MAP "x ".($DELTA-1)."\n";
	    print MAP "y ".($HEIGHT-$DELTA)."\n";
	    print MAP "hp ".($WIDTH-$DELTA-1)."\n";
	    print MAP "sp ".($DELTA)."\n";
	    print MAP "end\n";
	}
	if ($SOUTHEAST ne "") {
	    print MAP "arch exit\n";
	    print MAP "slaying $SOUTHEAST\n";
	    print MAP "x ".($WIDTH-$DELTA)."\n";
	    print MAP "y ".($HEIGHT-$DELTA)."\n";
	    print MAP "hp ".($DELTA)."\n";
	    print MAP "sp ".($DELTA)."\n";
	    print MAP "end\n";
	}
	if ($NORTHEAST ne "") {
	    print MAP "arch exit\n";
	    print MAP "slaying $NORTHEAST\n";
	    print MAP "x ".($WIDTH-$DELTA)."\n";
	    print MAP "y ".($DELTA-1)."\n";
	    print MAP "hp ".($DELTA)."\n";
	    print MAP "sp ".($HEIGHT-$DELTA-1)."\n";
	    print MAP "end\n";
	}

# Now lets do the edges.

	if ($NORTH ne "") {
	    $x=$DELTA;
	    while ($x < ($WIDTH-$DELTA)) {
	    	print MAP "arch exit\n";
		print MAP "slaying $NORTH\n";
		print MAP "x ".$x."\n";
		print MAP "y ".($DELTA-1)."\n";
		print MAP "hp ".$x."\n";
		print MAP "sp ".($HEIGHT-$DELTA-1)."\n";
		print MAP "end\n";
		$x=$x+1;
	    }
	}

	if ($SOUTH ne "") {
	    $x=$DELTA;
	    while ($x < ($WIDTH-$DELTA)) {
	    	print MAP "arch exit\n";
		print MAP "slaying $SOUTH\n";
		print MAP "x ".$x."\n";
		print MAP "y ".($HEIGHT-$DELTA)."\n";
		print MAP "hp ".$x."\n";
		print MAP "sp ".($DELTA)."\n";
		print MAP "end\n";
		$x=$x+1;
	    }
	}


	if ($WEST ne "") {
	    $y=$DELTA;
	    while ($y < ($HEIGHT-$DELTA)) {
	    	print MAP "arch exit\n";
		print MAP "slaying $WEST\n";
		print MAP "x ".($DELTA-1)."\n";
		print MAP "y ".$y."\n";
		print MAP "hp ".($WIDTH-$DELTA-1)."\n";
		print MAP "sp ".$y."\n";
		print MAP "end\n";
		$y=$y+1;
	    }
	}


	if ($EAST ne "") {
	    $y=$DELTA;
	    while ($y < ($HEIGHT-$DELTA)) {
		print MAP "arch exit\n";
		print MAP "slaying $EAST\n";
		print MAP "x ".($WIDTH-$DELTA)."\n";
		print MAP "y ".$y."\n";
		print MAP "hp ".($DELTA)."\n";
		print MAP "sp ".$y."\n";
		print MAP "end\n";
		$y=$y+1;
	    }
	}
	close MAP;
	$yc=$yc+1;
    }
    $xc=$xc+1;
}

# Second pass. Connected maps opened and primary map superimposed.

print STDOUT "Done with connecting, now superimposing split map.\n";
$xc=1;
$yc=1;
while ($xc<=$XM) {
    $yc=1;
    while ($yc<=$YM) {
	$NEWMAP=$MAPNAME."_".$xc."_".$yc.".new";
	$THISMAP=$MAPNAME."_".$xc."_".$yc;
	open (MAP, ">>$THISMAP") or die "unable to open new mapfile.";
#	open (CONMAP, "$THISMAP") or die "unable to open connected mapfile.";
	open (IMPMAP, "$MAPNAME") or die "unable to open superimposed mapfile.";
	$currentline=<IMPMAP>;
	print STDOUT "Now superimposing on map ".$THISMAP."\n";
# Discard header
	$headscan=1;
	while ($headscan) {
	    if ($currentline=~/end\n/) {
		$headscan=0;
	    }
	    $currentline=<IMPMAP>;
	}
# Read rest of file
	while ($currentline) {
#	    print STDOUT $currentline;
# Scan for and buffer archs within bounds.
	    while ($currentline ne "end\n" && $currentline) {

		if ($currentline=~/x /) {
		    ($florp, $px) = split //,$currentline,2;
		    $currentline="x ".($px-(($xc-1)*$WIDTH)+(($xc-1)*$DELTA*2))."\n";
		}
		if ($currentline=~/y /) {
		    ($florp, $py)=split //,$currentline,2;
		    $currentline="y ".($py-(($yc-1)*$HEIGHT)+(($yc-1)*$DELTA*2))."\n";
		}
		if ($currentline ne "x 0\n" && $currentline ne "y 0\n"){
		    $buf=$buf.$currentline;
		}
		$currentline=<IMPMAP>;
	    }
	    $buf=$buf.$currentline;
#	    print STDOUT $px.$py;
	    if ($px >= (($xc-1)*$WIDTH)-(($xc-1)*$DELTA*2) && $px < ($xc*$WIDTH)-(($xc-1)*$DELTA*2) && $py >= (($yc-1)*$HEIGHT)-(($yc-1)*$DELTA*2) && $py < ($yc*$HEIGHT)-(($yc-1)*$DELTA*2)) {
#		print STDOUT "In map: ".$THISMAP."\n";
		print MAP $buf;
#		print STDOUT ".";

	    }
# else {
#		print STDOUT "-";
#	    }
	    $px=0;
	    $py=0;
	    $buf="";
	    $currentline=<IMPMAP>;
	}
	close MAP;
	close CONMAP;
	close IMPMAP;
	$yc=$yc+1;
#	print STDOUT "\n";
    }
    $xc=$xc+1;
}
