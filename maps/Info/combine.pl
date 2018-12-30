#!/usr/bin/perl
# This script takes a bunch of world image files and combines them
# into one large image.  This should be run from the directory
# where all the images are.
# Note that this takes a while to run.  Probably pretty proportional
# to the the size of the target image.
# This is somewhat hacked for my usage - it presumes you run this
# from the top level of the maps directory (eg, contains world, scorn,
# etc directories).  This is smart enough to regen the image files
# that may be missing/out of date.

$DEST_WIDTH=3000;
$DEST_HEIGHT=3000;

$NUM_X=30;
$NUM_Y=30;

$START_X=100;
$START_Y=100;

$TILE_WIDTH = $DEST_WIDTH/$NUM_X;
$TILE_HEIGHT = $DEST_HEIGHT/$NUM_Y;
# This is the command to run the editor.  Really, it can be anything that
# outputs a .png file.  The %I and %O are substituted with actul
# path names.
$CFEDITOR="cd /export/home/crossfire/CFJavaEditor; java -Xmx128mb -classpath class/:lib/png.jar:lib/visualtek.jar cfeditor.CFJavaEditor -infile %I -outfile %O > /dev/null";

die ("No images directory - exiting\n") if (! -d "./images");
use Cwd;
$cwd = cwd();


# If we already have a combined image, then we only need to paste
# the new bits onto it, saving a bunch of time
if (! -f "images/combine.ppm") {
    system("ppmmake \\#000 $DEST_WIDTH $DEST_HEIGHT > /tmp/tmp.ppm");
    $first_run=1;
    print "Creating images for the first time.\n";
} else {
    system("cp images/combine.ppm /tmp/tmp.ppm");
    $first_run=0;
}

print "Processing.";
for ($x=0; $x<$NUM_X; $x++) {
    for ($y=0; $y<$NUM_Y; $y++) {
	print ".";
	$dx = $x + $START_X;
	$dy = $y + $START_Y;

	# These time values are the reverse in how you'd normally think about them - they
	# are the time (in days) since the fiel was last modified.  Thus, a file that hasn't
	# been modified in a long time has a high value, a file modified recently has
	# a low level.  
	$time1 = -M "images\/world_$dx\_$dy.png";
	$time2 = -M "world\/world_$dx\_$dy";
	if ($time1 > $time2) {
	    $cmd = $CFEDITOR;
	    $cmd =~ s#%I#$cwd/world/world_$dx\_$dy#;
	    $cmd =~ s#%O#$cwd/images/world_$dx\_$dy.png#;
	    system($cmd);
	    system("pngtopnm images/world_$dx\_$dy.png | pnmscale -xysize $TILE_WIDTH $TILE_HEIGHT > /tmp/ppm.tmp");
	    $sx = $x * $TILE_WIDTH;
	    $sy = $y * $TILE_HEIGHT;
	    system("pnmpaste /tmp/ppm.tmp $sx $sy /tmp/tmp.ppm > /tmp/tmp.ppm1");
	    unlink("/tmp/tmp.ppm");
	    rename("/tmp/tmp.ppm1", "/tmp/tmp.ppm");
	}
    }
}
system("mv /tmp/tmp.ppm images/combine.ppm");
print "\n";
