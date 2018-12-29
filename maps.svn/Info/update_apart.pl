#!/usr/bin/perl
#
# This map goes and updates all of the players apartment maps to be
# of the new naming convention.  This is basically derived from
# update_exits, but it also knows to rename the files before processing
# them.

# This takes the directory of the files you want to change.
# If you want to process your current working directory, do something
# like "update_apart.pl ./" - it needs that trailing slash to work
# right.

# The "SUBS" is for pathname substitution = put whatever name you want
# instead.

# Unique map files - mostly guilds, uses the @ format for /
# Put he first / in the guilds - otherwise, if you re-run the script,
# it will just keep renaming these, since a pathname component is
# put in front of these in the rename.
push(@SRC, "/guilds\@black_shield");
push(@DST, "brest\@black_shield");
push(@SRC, "/guilds\@damned_heretics");
push(@DST, "wolfsburg\@guilds\@damned_heretics");
push(@SRC, "/guilds\@dreaming_sage");
push(@DST, "navar_city\@guilds\@dreaming_sage");
push(@SRC, "/guilds\@drunken_barbarian");
push(@DST, "santo_dominion\@guilds\@drunken_barbarian");
push(@SRC, "/guilds\@guildhousesinc");
push(@DST, "scorn\@guilds\@guildhousesinc");
push(@SRC, "/guilds\@laughing_skull");
push(@DST, "pup_land\@guilds\@laughing_skull");
push(@SRC, "/guilds\@mailed_fist");
push(@DST, "scorn\@guilds\@mailed_fist");
push(@SRC, "/guilds\@poisoned_dagger");
push(@DST, "darcap\@darcap\@guilds\@poisoned_dagger");
push(@SRC, "/guilds\@purple_butterfly");
push(@DST, "pup_land\@guilds\@purple_butterfly");
push(@SRC, "/guilds\@smoking_cauldron");
push(@DST, "darcap\@darcap\@guilds\@smoking_cauldron");
push(@SRC, "/city\@");
push(@DST, "scorn\@");
# Per player unique maps use _ for path.
push(@SRC, "/_city_");
push(@DST, "_scorn_");


# Written by Mark Wedel (mwedel@sonic.net) 
# This borrows some amount of code from the map_info script written
# by Tero Haatanen <Tero.Haatanen@lut.fi>

# Name of the old map that we update exits on
# Note that this can be a regexp.

# OLD_MAP_STARTX/Y and OLD_MAP_ENDX/Y determine the range for the 
# updates.  For example, scorn/city was broken up on two of the
# map tiles, so this gets used to correspond that properly.
# you can use very large END values just to make sure the entire
# map is covered

# The list of values here are locations to update to and
# from.  I set it up this way with explicity value settings
# instead of initializing each as an array - I think this format
# makes it easier to see how everything relates.
#
# OLD_MAP_NAME: This is the path it tries to match in the slaying field.
# It can be a regexp.  When updating within a specific directory of
# a town, including the relative entries is possible.
# OLD_MAP_STARTX/Y and OLD_MAP_ENDX/Y is the range of spaces 
# that we process.  If the location is not in this range, it is unchanged.
# Note that you can have multiple entries with the same OLD_MAP_NAME
# value as long as they have different START and END coordinates.
# NEW_MAP_NAME is the name that will be put into the slaying field.
# NEW_MAP_OFFX/Y is the modification to the target location in
# the exit.

# Many maps are just called 'town', so this is only useful if you know
# 'town' is really referring to santo dominion.
$OLD_MAP_NAME[0]="(santo_dominion/town)";
$OLD_MAP_STARTX[0]=3;
$OLD_MAP_STARTY[0]=12;
$OLD_MAP_ENDX[0]=27;
$OLD_MAP_ENDY[0]=100;
$NEW_MAP_NAME[0]="/world/world_102_108";
$NEW_MAP_OFFX[0]=0;
$NEW_MAP_OFFY[0]=-9;

# Start of scorn updates
$OLD_MAP_NAME[1]=".*/city/city";
$OLD_MAP_STARTX[1]=10;
$OLD_MAP_STARTY[1]=0;
$OLD_MAP_ENDX[1]=100;
$OLD_MAP_ENDY[1]=100;
$NEW_MAP_NAME[1]="/world/world_105_115";
$NEW_MAP_OFFX[1]=-10;
$NEW_MAP_OFFY[1]=18;

# Start of scorn updates
$OLD_MAP_NAME[2]=".*/city/city";
$OLD_MAP_STARTX[2]=0;
$OLD_MAP_STARTY[2]=0;
$OLD_MAP_ENDX[2]=9;
$OLD_MAP_ENDY[2]=100;
$NEW_MAP_NAME[2]="/world/world_104_115";
$NEW_MAP_OFFX[2]=40;
$NEW_MAP_OFFY[2]=18;

# brest updates
$OLD_MAP_NAME[3]='(.+/brest|brest)';
$OLD_MAP_STARTX[3]=0;
$OLD_MAP_STARTY[3]=0;
$OLD_MAP_ENDX[3]=100;
$OLD_MAP_ENDY[3]=100;
$NEW_MAP_NAME[3]="/world/world_107_123";
$NEW_MAP_OFFX[3]=17;
$NEW_MAP_OFFY[3]=16;

# Start of navar city - bunch of these as navar city is actually
# spread across 4 world maps.  Only navar city uses the 'city1'
# name, so the regex is safe.
$OLD_MAP_NAME[4]='(.+/city1|city1)';
$OLD_MAP_STARTX[4]=15;
$OLD_MAP_STARTY[4]=13;
$OLD_MAP_ENDX[4]=100;
$OLD_MAP_ENDY[4]=100;
$NEW_MAP_NAME[4]="/world/world_122_117";
$NEW_MAP_OFFX[4]=-15;
$NEW_MAP_OFFY[4]=-13;

$OLD_MAP_NAME[5]='(.+/city1|city1)';
$OLD_MAP_STARTX[5]=15;
$OLD_MAP_STARTY[5]=0;
$OLD_MAP_ENDX[5]=100;
$OLD_MAP_ENDY[5]=12;
$NEW_MAP_NAME[5]="/world/world_122_116";
$NEW_MAP_OFFX[5]=-15;
$NEW_MAP_OFFY[5]=37;

$OLD_MAP_NAME[6]='(.+/city1|city1)';
$OLD_MAP_STARTX[6]=0;
$OLD_MAP_STARTY[6]=0;
$OLD_MAP_ENDX[6]=14;
$OLD_MAP_ENDY[6]=12;
$NEW_MAP_NAME[6]="/world/world_121_116";
$NEW_MAP_OFFX[6]=35;
$NEW_MAP_OFFY[6]=37;

$OLD_MAP_NAME[7]='(.+/city1|city1)';
$OLD_MAP_STARTX[7]=0;
$OLD_MAP_STARTY[7]=13;
$OLD_MAP_ENDX[7]=14;
$OLD_MAP_ENDY[7]=100;
$NEW_MAP_NAME[7]="/world/world_121_117";
$NEW_MAP_OFFX[7]=35;
$NEW_MAP_OFFY[7]=-13;

$OLD_MAP_NAME[8]='(.+/kundi_area|kundi_area)';
$OLD_MAP_STARTX[8]=0;
$OLD_MAP_STARTY[8]=0;
$OLD_MAP_ENDX[8]=100;
$OLD_MAP_ENDY[8]=100;
$NEW_MAP_NAME[8]="/world/world_109_126";
$NEW_MAP_OFFX[8]=13;
$NEW_MAP_OFFY[8]=17;

$OLD_MAP_NAME[9]='(.+/stoneville|stoneville)';
$OLD_MAP_STARTX[9]=0;
$OLD_MAP_STARTY[9]=0;
$OLD_MAP_ENDX[9]=13;
$OLD_MAP_ENDY[9]=100;
$NEW_MAP_NAME[9]="/world/world_102_127";
$NEW_MAP_OFFX[9]=36;
$NEW_MAP_OFFY[9]=1;

$OLD_MAP_NAME[10]='(.+/stoneville|stoneville)';
$OLD_MAP_STARTX[10]=14;
$OLD_MAP_STARTY[10]=0;
$OLD_MAP_ENDX[10]=100;
$OLD_MAP_ENDY[10]=100;
$NEW_MAP_NAME[10]="/world/world_103_127";
$NEW_MAP_OFFX[10]=-14;
$NEW_MAP_OFFY[10]=1;

$OLD_MAP_NAME[11]='(.+/darcap|darcap)';
$OLD_MAP_STARTX[11]=0;
$OLD_MAP_STARTY[11]=0;
$OLD_MAP_ENDX[11]=100;
$OLD_MAP_ENDY[11]=100;
$NEW_MAP_NAME[11]="/world/world_116_102";
$NEW_MAP_OFFX[11]=18;
$NEW_MAP_OFFY[11]=26;

$OLD_MAP_NAME[12]='(.+/piratetown|piratetown)';
$OLD_MAP_STARTX[12]=0;
$OLD_MAP_STARTY[12]=0;
$OLD_MAP_ENDX[12]=100;
$OLD_MAP_ENDY[12]=100;
$NEW_MAP_NAME[12]="/world/world_128_109";
$NEW_MAP_OFFX[12]=12;
$NEW_MAP_OFFY[12]=0;

$OLD_MAP_NAME[13]='(.+/portjoseph|portjoseph)';
$OLD_MAP_STARTX[13]=0;
$OLD_MAP_STARTY[13]=0;
$OLD_MAP_ENDX[13]=100;
$OLD_MAP_ENDY[13]=100;
$NEW_MAP_NAME[13]="/world/world_101_114";
$NEW_MAP_OFFX[13]=7;
$NEW_MAP_OFFY[13]=22;

$OLD_MAP_NAME[14]='(.+/tortola|tortola)';
$OLD_MAP_STARTX[14]=0;
$OLD_MAP_STARTY[14]=0;
$OLD_MAP_ENDX[14]=100;
$OLD_MAP_ENDY[14]=100;
$NEW_MAP_NAME[14]="/world/world_100_116";
$NEW_MAP_OFFX[14]=16;
$NEW_MAP_OFFY[14]=6;


$VERBOSE=0;
$error=0;
for ($i=0; $i<=$#OLD_MAP_NAME; $i++) {
  if ((($OLD_MAP_STARTX[$i] + $NEW_MAP_OFFX[$i]) < 0) || 
    (($OLD_MAP_STARTY[$i] + $NEW_MAP_OFFY[$i]) < 0 )) {
	print "oldmap $OLD_MAP_NAME[$i] ($OLD_MAP_STARTX[$i], $OLD_MAP_STARTX[$i] will result in negative destination coordinates.\n";
	$error=1;
  }
}
# Basically, we want to check all the values and then exit.
exit(1) if ($error);

die("Usage: <directory>\n") if ($#ARGV < 0);
&maplist("$ARGV[0]");

while ($file = shift (@maps)) {
    $newfile = $file;
    for ($xyz=0; $xyz <= $#SRC; $xyz++) {
	$newfile =~ s#$SRC[$xyz]#$DST[$xyz]#;
    }
    # 
    if ($newfile ne $file) {
	if (-f $newfile) {
	    print STDERR "$newfile exists.  Overwrite? ";
	    # update_map mucks with $/ value, so reset 
	    $/ = "\n";
	    $ans = <STDIN>;
	    if ($ans =~ /^(y|Y)/) {
		unlink($newfile);
		print STDERR "renaming $file -> $newfile\n";
		rename($file, $newfile)
	    } else {
		$file = $newfile;   # so it still gets updated 
	    }
	}
	else {
	    print STDERR "renaming $file -> $newfile\n";
	    rename($file, $newfile)
	}
    }
    &updatemap($newfile);
}

exit;

# return table containing all objects in the map
sub updatemap {
    local ($m, $made_change=0);
    $last = "";
    $parent = "";
    $file = shift;
    
    # Note that $/ is the input record seperator.  By changing
    # this to \nend\n, it means that when we read from the file,
    # we basically read an entire arch at the same time.  Note that
    # given this, $ in regexps matches this value below, and not
    # a newline.  \n should generally be used instead of $ in
    # regexps if you really want the end of line.
    # Similary, ^ matches start of record, which means the arch line.

    $/ = "\nend\n";
    if (! open (IN, $file)) {
	print "Can't open map file $file\n";
	return;
    }
    $_ = <IN>;
    if (! /^arch map\n/) {
	# print "Error: file $file isn't mapfile.\n";
	return;
    }
    if (! open(OUT, ">$file.new")) {
	print "Can't open output file $file.new\n";
	return;
    }
    print OUT $_;
    if ($VERBOSE) {
	    print "Testing $file, ";
	    print /^name (.+)$/ ? $1 : "No mapname";
	    print ", size [", /^x (\d+)$/ ? $1 : 16;
	    print ",", /^y (\d+)/ ? $1 : 16, "]";
    
	    if (! /^msg$/) {
		print ", No message\n";
	    } elsif (/(\w+@\S+)/) {
		print ", $1\n";
	    } else {
		print ", Unknown\n";
	    }
	    $printmap=0;
    }
    else {
	$name=  /^name (.+)$/ ? $1 : "No mapname";
	$x=  /^x (\d+)$/ ? $1 : 16;
	$y= /^y (\d+)/ ? $1 : 16;
	$mapname="Map $file, $name, size [$x, $y]\n" ;
	$printmap=1;
    }

    while (<IN>) {
	if (($m = (@_ = /^arch \S+\s*$/g)) > 1) {
	    $parent = /^arch (\S+)\s*$/;
	    print OUT $_;

	    # Object has an inventory.  Just read through until we get
	    # an end
	    while (<IN>) {
		last if (/((.|\n)*end\n)(arch (.|\n)*\nend\n)/);
		print OUT $_;
	    }
	    $parent="";
	    # Objects with inventory should not contain exits, so
	    # do not need to try and process them.  Likewise, the objects
	    # in the inventory should not contain exits.
	} else { 
	    for ($i=0; $i<=$#OLD_MAP_NAME; $i++) {
		if (m#\nslaying $OLD_MAP_NAME[$i]\n#) {
		    $destx = /\nhp (\d+)\n/ ? $1 : 0;
		    $desty = /\nsp (\d+)\n/ ? $1 : 0;
		    if ($destx >= $OLD_MAP_STARTX[$i] && $destx <= $OLD_MAP_ENDX[$i]  &&
			$desty >= $OLD_MAP_STARTY[$i] && $desty <= $OLD_MAP_ENDY[$i]) {
			# Ok.  This exit matches our criteria.  Substitute in
			# the new values
			s/slaying $OLD_MAP_NAME[$i]\n/slaying $NEW_MAP_NAME[$i]\n/;
			$destx += $NEW_MAP_OFFX[$i];
			$desty += $NEW_MAP_OFFY[$i];
			s/\nhp \d+\n/\nhp $destx\n/;
			s/\nsp \d+\n/\nsp $desty\n/;
			$made_change=1;
		    }
		}
		elsif (m#\nslaying /city/apartment#) {
			s#/city/apartment#/scorn/apartment#;
			$made_change=1;
		}
	    }
	    print OUT $_;
	} # else not an object with inventory
    } # while <IN> LOOP
    close (IN);
    close(OUT);
    if ($made_change) {
	print "$file has changed\n";
	unlink($file);
	rename("$file.new", $file);
    }
    else {
	unlink("$file.new");
    }
}

# @maps contains all filenames
sub maplist {
    local ($dir, $file, @dirs) = shift;

    opendir (DIR , $dir) || die "Can't open directory : $dir\n";
    while ($file = readdir (DIR)) {
	next if ($file eq "." || $file eq ".." || $file eq "CVS");

	$file = "$dir/$file";
	next if (-l $file);	# don't process symbolic links 
	push (@dirs, $file) if (-d $file);
	push (@maps, $file) if (-f $file);
    }
    closedir (DIR);

    # recursive handle sub-dirs too
    while ($_ = shift @dirs) {
	&maplist ($_);
    }
}

