#!/usr/bin/perl
#
# This script goes and fixes the *_style names for random maps.
# It is aimed at the mlab maps.  the original mlab maps used
# uppercase file names, which were quite ugly, and also put the
# new styles with old styles, which isn't good when it comes to
# random styles.  Instead, I put those style maps into their own
# subdirectory so they should only show up on mlab maps.
#

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
    
    if (! open (IN, $file)) {
	print "Can't open map file $file\n";
	return;
    }
    if (! open(OUT, ">$file.new")) {
	print "Can't open output file $file.new\n";
	return;
    }
    if ($VERBOSE) {
	    print "Testing $file, ";
    }
    while (<IN>) {
	if (/(.*style) (MLAB.*)/) {
	    $style= $1 . "style";
	    $dest = "mlab/" . $2;
	    $dest =~ tr /A-Z/a-z/;
	    print OUT "$style $dest\n";
	    $made_change=1;
	} else {
	    print OUT $_;
	}
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

