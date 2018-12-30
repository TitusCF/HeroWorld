#!/usr/bin/perl
eval 'exec perl -S $0 "$@"'
    if $running_under_some_shell;
			# this emulates #! processing on NIH machines.
			# (remove #! line above if indigestible)

eval '$'.$1.'$2;' while $ARGV[0] =~ /^([A-Za-z_0-9]+=)(.*)/ && shift;
			# process any FOO=bar switches

# makeps - make Postscript-files of the archetypes listed in text file whose
# filename is passed in 'input'
# Variables passed in:
#   archdir - root of crossfire-src, with a trailing slash
#   libdir  - where archetypes etc. is found

$[ = 1;			# set array base to 1
$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

$size=0.4;
$IMAGE_SIZE=32;		# Size of PNG images

if ($output eq "tex") { $BG="\\#ffffff"; }
else { $BG="\\#ab0945"; }

# Set colour to 1 if you want colour postscript.
$colour = 0;
# IF you have giftrans installed and want transparent gifs, set
# appropriately.  IT looks much nicer if you can do it.
$giftrans = 0;


$bmaps = $libdir . '/bmaps';
$bmappaths = $libdir . '/bmaps.paths';

open(BMAPS,"<".$bmappaths) || die("Can't open $bmappaths");
while (<BMAPS>) {
    ($f1,$f2) = split;
    if ($f1 ne '#') {
	# A bit tricky.  We first substitute the first .
	# (./arch to ^/arch), so that the second substitute
	# puts the 'base' portion in the name, and then we
	# put the first . back in place.
	$f2 =~ s/\./\^/;
	$f2 =~ s/\./\.base\./;
	$f2 =~ s/\^/\./;
	$bmappath{$f1} = $f2;
    }
}
close(BMAPS);

open(BMAPS,"<".$bmaps);
while (<BMAPS>) {
    ($f1,$f2) = split;
    if (defined $bmappath{("\\".$f1)}) {
	$bmap{$f2} = $bmappath{("\\".$f1)};
    }
}
close(BMAPS);

# An array listing which archetypes files need fixing, the value
# is the file where it is used. There must be at least one character
# between the ~~spec~~'s.


open(IN,"<".$input) || die("can not open $input\n");
while (<IN>) {
    @flds = split(/~~/);
    $work_todo = 1;
    $i = 2;
    while ($flds[$i] ne "") {
	        $makeps{$flds[$i]} = 0;
		$i += 2;
    }
}
close(IN);

if ($output ne "png") {
    # An array to reduce the size of the bitmap exponentially.
    # A 4x8 bitmap will be reduced to 60% of its full size.
    if ($work_todo) {
	$size_mul{1} = 1;
	for ($i = 2; $i <= 12; $i++) {# Max input is 12x12, a *large* bitmap ;-)
	    $size_mul{$i} = $size_mul{$i - 1} * 0.9;
        }
    }
}


$More = 0;
print STDERR "starting to process $inarch\n";
open(IN,"<".$inarch) || die("could not open $inarch\n");
line: while (<IN>) {
    chomp;	# strip record separator
    @Fld = split(/ /, $_, 2);
    if ($Fld[1] eq 'Object') {
	if ($interesting) {
	    $faces{$X, $Y} = $face;
	    if (!$More && $makeps{$obj} != 1) {
		$makeps{$obj} = &assemble();
	    }
	}

	# Get ready for next archetype
	if (!$More) {
	    $xmin = $xmax = $ymin = $ymax = 0;
	    $obj = $Fld[2];
	    $interesting = defined $makeps{$obj};
	}
	$X = $Y = 0;
	$More = 0;
    }

    if ($Fld[1] eq 'face') {
	$face = $Fld[2];
    }
    if ($Fld[1] eq 'x') {
	$X = $Fld[2];
	if ($X > $xmax) {	#???
	    $xmax = $X;
	}
	elsif ($X < $xmin) {	#???
	    $xmin = $X;
	}
    }
    if ($Fld[1] eq 'y') {
	$Y = $Fld[2];
	if ($Y > $ymax) {	#???
	    $ymax = $Y;
	}
	elsif ($Y < $ymin) {	#???
	    $ymin = $Y;
	}
    }
    if ($Fld[1] eq 'More') {
	$More = 1;
    }
    if ($Fld[1] eq 'msg') {
	do {
	    $_ = <IN>;
	    @Fld = split;
	}
	while ($Fld[1] ne 'endmsg');
    }
}
close(IN);

# Remember to check the last archetype also...
if ($interesting) {
    $faces{$X, $Y} = $face;
    if ($makeps{$obj} != 1) {
	$makeps{$obj} = &assemble();
    }
}

system('rm -f work.pbm tmp.pbm empty.pbm');
# clean up a little
system("pbmmake -white $IMAGE_SIZE $IMAGE_SIZE > empty.pbm");

# We've created a number of Postscript-files - now we need to
# patch the filenames and sizes into the TeX-files.

$, = '';
open(IN,"<".$input);
while (<IN>) {
    @Fld = split(/~~/);
    if ($#Fld > 1) {
	for ($i = 2; $i <= $#Fld; $i += 2) {
	    if (defined $makeps{$Fld[$i]}) {
    			$Fld[$i] = $makeps{$Fld[$i]};
	    }
	}
    }
    print @Fld;
}
close(IN);


sub assemble {
    local($w, $h, $ppm, $buff, $i, $j, $bmap_file, $ps_file) = @_;
    my($one_image)=0;

    $bmap_file = $archdir.$bmap{$faces{0,0}}.".png";
    if ($output eq "tex") {$ps_file = $faces{0, 0} . '.ps';     }
    elsif ($output eq "png") { $ps_file = $faces{0, 0} . '.png'; }
    elsif ($output eq "pdf") {
        $tmp = $faces{0, 0};
        $tmp =~ s/\./-/gi;
        $ps_file = $tmp . '.png';
    } else { $ps_file = $faces{0, 0} . '.gif'; }

    $ps_file =~ s/[_ ]/-/g;

    # We don't need to manipulate the files, so just do hard links - much
    # faster, and also doesn't use space.
    if (($output eq "png") || ($output eq "pdf")) {
	link($bmap_file, $ps_file);
	if ($output eq "png") {
	   $ps = "<img src=$ps_file>";
        } else {
            $ps = "\\includegraphics[scale=0.5]{" . $ps_file . "}";
        }
	return $ps;
    }

    $w = $xmax - $xmin + 1;
    $h = $ymax - $ymin + 1;

    # with big image support, we don't need to assemble images.  But not all
    # images are big image - so we do a simple check - see if the face for the
    # first and last piece are the same - if so, presume this is a big image
    if ($archdir.$bmap{$faces{0,0}} eq $archdir.$bmap{$faces{$w-1,$h-1}}) { $one_image=1; }

    if (! -e $ps_file) {
	if ((($w == 1) && ($h == 1)) || $one_image) {
	    # Maybe ln -s instead?
	    if ($output eq "tex") {
		if ($colour) {	system("pngtopnm -mix -background $BG $bmap_file | pnmtops -noturn -nosetpage > $ps_file"); }
		else {	system("pngtopnm -mix -background $BG $bmap_file | pnmdepth 255 | ppmtopgm | pnmtops -noturn -nosetpage> $ps_file"); }
	    }
	    elsif ($giftrans) {
		system("pngtopnm -mix -background $BG $bmap_file | ppmtogif | giftrans -t $BG $ppm > $ps_file");
	    } else {
		system("pngtopnm -mix -background $BG $bmap_file | ppmtogif > $ps_file");
	    }
	}
	else {
	    $ppm = sprintf('%dx%d.ppm', $w, $h);
	    print STDERR "$ppm\n";
	    if (! -e $ppm) {
		print STDERR
		      "pnmscale -xsc $w -ysc $h < empty.pbm | pgmtoppm white > $ppm\n";

		system(sprintf('pnmscale -xsc %d -ysc %d < empty.pbm | pgmtoppm white > %s',
		$w, $h, $ppm));
	    }

	    system("cp $ppm work.ppm");
	    $ppm = "work.ppm";

	    for ($i = $xmin; $i <= $xmax; $i++) {
		for ($j = $ymin; $j <= $ymax; $j++) {
		    print STDERR
			 'Processing x ' . $bmap{$faces{$i, $j}};
		    $valx = ($i - $xmin) * $IMAGE_SIZE;
		    $valy = ($j - $ymin) * $IMAGE_SIZE;
#		    print STDERR "pngtopnm -background #ABCD01239876 $archdir$bmap{$faces{$i,$j}}.png > tmp.ppm\n";
		    system("pngtopnm -mix -background $BG $archdir$bmap{$faces{$i,$j}}.png > tmp.ppm");
		    system("pnmpaste tmp.ppm $valx $valy $ppm > tmp2.ppm");
		    rename("tmp2.ppm", $ppm);
		}
	    }
	    if ($output eq "tex") {
		if ($colour) {	system("pnmtops -noturn $ppm> $ps_file"); }
		else {	system("pnmdepth 255 $ppm | ppmtopgm | pnmtops -noturn > $ps_file"); }
	    }
	    elsif ($giftrans) {
		system("ppmtogif $ppm | giftrans -t $BG > $ps_file");
	    } else {
		system("ppmtogif $ppm > $ps_file");
	    }
	}
    }
    $mul = $size_mul{int(sqrt($w * $h))} * $size;
    if ($output eq "tex") {
	if ($h == 1) {
	    $ps = sprintf "\\psfig{figure=$ps_file,width=%0.2fcm,height=%0.2fcm}", $w * $mul, $h * $mul;
	} else {
	    $ps = sprintf "\\raisebox{-%0.2fcm}{\\psfig{figure=$ps_file,width=%0.2fcm,height=%0.2fcm}}", ($h-1) * $mul, $w * $mul, $h * $mul;
	}
    } else {
	    $ps = "<img src=$ps_file>";
    }
    $ps;
}
