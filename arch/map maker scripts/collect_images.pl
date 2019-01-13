#!/usr/bin/perl
#
# this script goes through and collects the various image sets into crossfire-x.png
# files, where x is the numbers from the image_info file.  This
# collects all the images at once, whether or not they have
# been changed or not.

# This script collects all the images.  If given a -archive option, it
# does some additional work - checksumming the images, making a bmaps.client
# file, and then tarring that data up.  This data can then be used on the
# client as a fast way to 'bootstrap' the clients images.

use FileHandle;

die("No arch directory - will not rebuild $mode image file") if (! -e "arch");

$archive = 0;
$TMPDIR="/tmp";

# What we will call the collection of images.
$ARCHNAME="crossfire-images";
$DESTDIR="$TMPDIR/$ARCHNAME";

# Maximum expected file 
$MAXFILESIZE=100000;

if ($ARGV[0] eq "-archive") {
	$archive =1;
	print "Will generate appropriate files for image archive\n";
	die("$DESTDIR already exists - remove if you really want to remake the images") if (-d $DESTDIR);
	die("$0: unable to mkdir $DESTDIR: $1\n") if (!mkdir($DESTDIR, 0755));
}

open(IMAGEINFO,"image_info") || die("Can't open image_info file: $!\n");
while (<IMAGEINFO>) {
    # Ignore lines that start with comments or just empty lines
    next if /^#/;
    next if /^\s*$/;
    ($setnum, $ext, @rest) = split /:/;
    # We don't actually need that data in this script, but may as well do sanity
    # checking.
    if ($#rest != 4) {
	print STDERR "image_info: line is corrupt:\n$_";
    }
    if ($extension[$setnum]) {
	print STDERR "Warning: set $setnum is duplicated in image_info file\n";
    }
    $extension[$setnum] = $ext;
}
close(IMAGEINFO);


for ($count=0; $count<=$#extension; $count++) {
    $ESRV[$count] = new FileHandle;
    $fh = $ESRV[$count];
    open($fh, ">crossfire.$count") ||
	die("Can't open crossfire.$count for write: $!\n");
	binmode( $fh );
}

open(BMAPS,"bmaps.paths") || die("Can't open bmaps.paths: $!\n");
$_ = <BMAPS>;
while(<BMAPS>) {
    chop;

    # we need to insert the extension for the images between the name
    # and the number (.171 or whatever) extension, so split on that.

    die("Unknown line: '$_'\n") unless /^\\(\d{5})\s+(\S+)\.(\w\w\w)$/o;
    $num = $1;
    $file = $2;
    $file1 = $3;

    print "$num $file\n" if ($num % 500) == 0 ;
    # This probably isn't the most efficient way to do this if a 
    # large number of images are added, as we try to open each
    # instance.
    # OTOH, we are doing one directory
    # at a time, so we should be hitting the DNLC at a very high
    # rate.

    for ($count=0; $count<=$#extension; $count++) {
	$filename = "$file.$extension[$count].$file1.png";
	$fh = $ESRV[$count];

	$length = -s "$filename";
	if (open(FILE,"$filename")) {
	    binmode( FILE );
	    print $fh "IMAGE $num $length $file.$file1\n";
	    print "Error reading file $filename" if (!read(FILE, $buf, $length));
	    $position = tell $fh;
	    print $fh $buf;
	    close(FILE);

	    if ($archive) {
		# Now figure out the checksum
		# Same as what is used for the client/server - code basically
		# taken write form that.
		$sum = 0;
		for ($i=0; $i<$length; $i++) {
		    if ($sum & 01) {
			$sum = ($sum >> 1) | 0x80000000;
		    } else {
			$sum >>= 1;
		    }
		    $sum += ord(substr $buf, $i, 1);
		    $sum &= 0xffffffff;
		}
		# Do some name translation to figure out our output file name.
		@comps = split /\//, $file;
		$destfile = $comps[$#comps];
		push @csums, "$destfile.$file1 $sum crossfire.$extension[$count]\@$position:$length\n";
	    } # if archive
	}
	elsif ($count==0) {
	    # set 0 should have all the sets
	    print "Error: Image $filename not found for set 0!\n";
	}
	
    }
}
for ($count=0; $count<=$#extension; $count++) {
    close($ESRV[$count]);
}
close(BMAPS);

if ($archive) {
    open(OUT,">$DESTDIR/bmaps.client") || die("Can not open $DESTDIR/bmaps.paths\n");
    print OUT sort @csums;
    close(OUT);
    open(OUT,">$DESTDIR/README") || die("Can not open $DESTDIR/README\n");
    print OUT "These image files for the client should be located in\n";
    print OUT "\$prefix/share/crossfire-client.  \$prefix is the location given in the -prefix\n";
    print OUT "option when configure is run to build the client.  The default is /usr/local.\n";
    print OUT "In that case these files should be put in /usr/local/share/crossfire-client\n";
    print OUT "The client will print a messgae if it is unable to find the image information\n";
    print OUT "with the location is looked for them.\n";
    close(OUT);

    for ($count=0; $count<=$#extension; $count++) {
	system("cp crossfire.$count $DESTDIR/crossfire.$extension[$count]");
    }
    system("cd $DESTDIR; tar cf $TMPDIR/$ARCHNAME.tar .");
    system("mv $TMPDIR/$ARCHNAME.tar ../");
    system("rm -rf $TMPDIR/$ARCHNAME");
}
