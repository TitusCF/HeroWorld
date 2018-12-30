#!/usr/bin/perl
#
# This script goes through and updates the exits for maps.
# First pass, so a lot of the options need to be set in this
# script.  It will search all directories in and below your current
# working directory, so run from the directory you want to update.

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


&maplist(".");

while ($file = shift (@maps)) {
    &updatemap;
}


exit;

# return table containing all objects in the map
sub updatemap {
    local ($m, $made_change=0);
    $last = "";
    $parent = "";
    
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
	print "Error: file $file isn't mapfile.\n";
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
	$made_change=1 if (s/Lake_Country/lake_country/g);
        print OUT $_;
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
	push (@dirs, $file) if (-d $file);
	push (@maps, $file) if (-f $file);
    }
    closedir (DIR);

    # recursive handle sub-dirs too
    while ($_ = shift @dirs) {
	&maplist ($_);
    }
}

