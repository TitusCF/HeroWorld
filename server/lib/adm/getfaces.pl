#!/usr/bin/perl

#    getfaces.pl, a script to retrive lists of specific faces from crossfire
#                 archtypes
#
#    Copyright (C) 2005 Alexander Schultz
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#    The author can be reached via e-mail to aldragon@gmail.com


use Getopt::Long;
use File::Find;

$result = GetOptions ("inclusive!"  => \$inclusiveflags, "showfile!"  => \$showfileflag, "showobject!"  => \$showobjectflag, "duplicate!"  => \$duplicateflag, "nopass"  => \$nopass, "alive"  => \$alive, "player"  => \$player, "monster"  => \$monster, "floor"  => \$floor, "spelleffect"  => \$spelleffect, "searchname=s"  => \$searchstring, "searchbody=s"  => \$searchbstring, "race=s"  => \$racestring, "slay=s"  => \$slaystring, "help|h" => \$showhelp, "stdin=s" => \$stdinlimit, "imagefile" => \$imagefileflag, "imagemagick" => \$imagemagickflag, "imagelabels" => \$imagelabelsflag, "type=i", => \$checktype, "subtype=i", => \$checkstype, "exit" => \$exitflag, "teleporter" => \$teleflag, "weapon" => \$weapflag, "bow" => \$bowflag, "arrow" => \$arrowflag, "armour" => \$armourflag, "currency" => \$currencyflag);
if (!$result) { usage(); }
if ($showhelp) { help(); }
if (!($stdinlimit=~/limit/i) and !($stdinlimit=~/extend/i) and !($stdinlimit=~/exclude/i) and $stdinlimit) { print "--stdin must be either 'limit', 'exclude' or 'extend'.\n"; usage(); }

$curpath = "../arch";
if ($searchstring) { $namere = qr/$searchstring/i; }
if ($searchbstring) { $bodyre = qr/$searchbstring/i; }
if ($racestring) { $racere = qr/$racestring/i; }
if ($slaystring) { $slayre = qr/$slaystring/i; }
if ($checktype) { $typenum = int($checktype); }
if ($checkstype) { $stypenum = int($checkstype); }
if ($ARGV[0]) { $curpath=$ARGV[0]; }
%animfaces = ();
%unique = ();
%unique2 = ();
%stdinlist = ();
%pngloc = ();
@outfacefiles = ();
$fcount = 0;


#create an internal database of animations defined in .face files to reference to
File::Find::find({wanted => \&facehandle, follow => 1, no_chdir => 1 }, $curpath);


if ($stdinlimit) {
	while (<>) {
		$_ =~ s/\s+$//;
		$oldface = $_;
		if ($imagefileflag or $imagemagickflag) {$_ =~ s/\./\.base\./; $_=$_.".png";}
		$stdinlist{$_}++;
		if ($stdinlimit=~/extend/i) {
			if (!$imagemagickflag) {
				print "$_\n";
			}
			$unique{$_}++;
			$fcount++;
			if ($imagelabelsflag) {$imprams="-label ".$_." ";} else {$imprams="";}
			$outfacefiles[$fcount] = $imprams.$pngloc{$_};
		}
		if ($stdinlimit=~/exclude/i) { $unique{$_}++; }
	}
}


#look through all .arc files for data
File::Find::find({wanted => \&archhandle, follow => 1, no_chdir => 1 }, $curpath);

if ($imagemagickflag) { imagemagickdisp(); }

exit;

sub usage {
	$usagemessage = "Usage: getfaces.pl [OPTION]... [PATH]
Try `./getface.pl --help' for more information.
";
	die $usagemessage;
}

sub help {
	$usagemessage = "Usage: getfaces.pl [OPTION]... [PATH]
List faces used by archtypes that fit given criteria.

Settings:
  --showfile               before each face, list the .arc that is using it
  --showobject             before each face, list the name of the object
                           that is using it
  --duplicate              show duplicate entries of faces for each object
                           using the face
  --inclusive              normally the logic of the criteria is exclusive,
                           this option makes the criteria behave inclusively
  --imagefile              insert tileset data (assuming tileset 'base') and
                           .png extension into the image name such that it can
                           be used to easily locate the image file.
  --stdin=MODE             read data from stdin. MODE can either be 'extend'
                           'exclude' or 'limit'. The limit mode means that we
                           will only output faces that are are listed in what
                           stdin. The extend mode prepends stdin to the output
                           and will not repeat those faces again unless
                           duplicate is enabled. Exclude will cause it to not
                           output faces that appear in stdin, but exclude will
                           not do anything at all if duplicate is enabled. Note
                           that this will hang the program is nothing is passed
                           to stdin and that include has no effect on this.

Graphical Display:
  --imagemagick            Use the external imagemagick program to create and
                           display (defaultly using eog) a montage of outputted
                           faces. Stores temporary montage file in
                           /tmp/cffaces.png normally.
  --imagelabels            When using the imagemagick flag, this will cause
                           labels for the face to be inserted under the face.
                           Note that this does nothing outside of imagemagick
                           mode, and the rendering of the labels can take a
                           *long* time.

Criteria:
  --nopass                 Only list faces for objects with move_block set
  --floor                  Only list faces for objects with is_floor 1
  --alive                  Only list faces for objects with alive 1 (note that
                           this is not an accurate way to detect monsters or
                           players)
  --player                 Only list faces for objects that are players
  --monster                Only list faces for objects that are monsters
  --spelleffect            Only list faces for objects that are spelleffects
  --exit                   Only list faces for objects that are exits
  --teleporter             Only list faces for objects that are teleporters
  --weapon                 Only list faces for objects that are weapons
  --bow                    Only list faces for objects that are bows
  --arrow                  Only list faces for objects that are arrows
  --weapon                 Only list faces for objects that are some type of
                           armour
  --currency               Only list faces for objects that are money or gems
  --type=NUM               Only list faces for objects which have a type of NUM
  --subtype=NUM            Only list faces for objects which have a subtype of
                           NUM
  --race=PATTERN           Only list faces for objects with a race that
                           matches PATTERN
  --slay=PATTERN           Only list faces for objects with a slaying that
                           matches PATTERN
  --searchname=PATTERN     Only list faces for objects with a name that
                           matches PATTERN
  --searchbody=PATTERN     Only list faces for objects with a object data that
                           matches PATTERN

PATH is the path of the archtypes directory, and if no PATH is given it
defaults to ../arch
PATTERN is always in the form of a perl regexp, though plaintext usually
works too.
";
	die $usagemessage;
}

sub archhandle {

if (/^.*\.arc\z/s) {
	open (ARCHDATA, $File::Find::name) || die "Couldn't open $File::Find::name \n";
	while (<ARCHDATA>) {
		@words =split(/ /);
		if ($_ =~ /^mina/) { $countframes=0; $animcount=0; }
		if (($countframes==1) && ($_ =~ /^facings /)) {next;}
		if ($countframes==1) {
			$animcount++;
			#skip the first face of animation for gates if we're looking for nopass,
			#because that's the fully open frame.
			if ((($itype =~ /^91$/) or ($itype =~ /^26$/)) and ($animcount==1) and $nopass) {next;} else {
				$_ =~ s/\s+$//; $_=$_."\n";
				$faces = $faces.$_;
				next;
			}
		}
		if ($_ =~ /^More/) { $morestatus=1; }
		if ($_ =~ /^Object /) { chomp($name=$words[1]); $obstatus=1; $doneobject=0;
			#reset variables unless "More" just happened (as in some multi-tile monsters)
			if (not $morestatus) {
				$ialive=0;
				$imonster=0;
				$ifloor=0;
				$iteardown=0;
				$itype="";
				$istype="";
				$race="";
				$slay="";
				$face="";
				$pname="";
				$bodyinfo="";
			}
			$animcount=0;
			$inopass=0;
			$faces="";
		}
		if ($_ =~ /^name /) { chomp($pname=$words[1]); }
		if ($_ =~ /^end$/) { $newob=1; $obstatus=0; $countframes=0; $doneobject=1; }
		if ($_ =~ /^anim$/) { $countframes=1; $hasobject=1 }
		if ($_ =~ /^move_block [^0]/) {$inopass=1;}
		if ($_ =~ /^alive 1/) {$ialive=1;}
		if ($_ =~ /^is_floor 1/) {$ifloor=1;}
		if ($_ =~ /^monster 1/) {$imonster=1;}
		if ($_ =~ /^no_pass 0/) {$inopass=0;}
		if ($_ =~ /^alive 0/) {$ialive=0;}
		if ($_ =~ /^is_floor 0/) {$ifloor=0;}
		if ($_ =~ /^monster 0/) {$imonster=0;}
		if ($_ =~ /^type /) { $words[1] =~ s/\s+$//; $itype=$words[1]; }
		if ($_ =~ /^subtype /) { $words[1] =~ s/\s+$//; $istype=$words[1]; }
		if ($_ =~ /^race /) { $words[1] =~ s/\s+$//; $race=$words[1]; }
		if ($_ =~ /^slaying /) { $words[1] =~ s/\s+$//; $slay=$words[1]; }
		if ($_ =~ /^tear_down 1/) {$iteardown=1;}
		if (($_ =~ /animation /) && ($obstatus==1)) { $words[1] =~ s/\s+$//; $faces=$faces."".$animfaces{$words[1]}.""; }
		if ($_ =~ /^face/) { $words[1] =~ s/\s+$//; chomp($face=$words[1]); }
		if (($obstatus==1) and not ($_ =~ /^end$/)) { $bodyinfo=$bodyinfo.$_; }
		next if ($newob==0);
		if ($doneobject>=1) {
			#we're done with an object.
			$faces = $faces."$face\n";
			#check if it fits critera

			if ($inclusiveflags) {
				$nopasscheck = ($inopass and $nopass);
				$monstercheck = ((($imonster and not ($iteardown or ($itype =~ /^62$/) or ($itype =~ /^101$/)))) and $monster);
				$alivecheck = ($ialive and $alive);
				$playercheck = (((($itype =~ /^37$/) or ($itype =~ /^1$/)) or (($itype =~ /^74$/) and ($istype =~ /^31$/))) and $player);
				$bsearchcheck = (($bodyinfo =~ /$bodyre/) and $searchbstring);
				$searchcheck = ((($name =~ /$namere/) or ($pname =~ /$namere/)) and $searchstring);
				$racecheck = (($race =~ /$racere/) and $racestring);
				$slaycheck = (($slay =~ /$slayre/) and $slaystring);
				$floorcheck = ($ifloor and $floor);
				$spellcheck = (($itype =~ /^102$/) and $spelleffect);
				$typecheck1 = (($itype == $typenum) and $checktype);
				$typecheck2 = (($istype == $stypenum) and $checkstype);
				$exitcheck = (($itype =~ /^66$/) and $exitflag);
				$telecheck = (($itype =~ /^41$/) and $teleflag);
				$weapcheck = (($itype =~ /^15$/) and $weapflag);
				$bowcheck = (($itype =~ /^14$/) and $bowflag);
				$arrowcheck = (($itype =~ /^13$/) and $arrowflag);
				$armourcheck = ((($itype =~ /^99$/) or ($itype =~ /^100$/) or ($itype =~ /^104$/) or ($itype =~ /^87$/) or ($itype =~ /^113$/) or ($itype =~ /^34$/) or ($itype =~ /^16$/) or ($itype =~ /^133$/)) and $armourflag);
				$currcheck = ((($itype =~ /^36$/) or ($itype =~ /^60$/)) and $currencyflag);
				$criteria = ($nopasscheck or $monstercheck or $alivecheck or $playercheck or $searchcheck or $bsearchcheck or $racecheck or $slaycheck or $floorcheck or $spellcheck or $typecheck1 or $typecheck2 or $exitcheck or $telecheck or $weapcheck or $bowcheck or $arrowcheck or $armourcheck or $currcheck);
			} else {
				$nopasscheck = ($inopass or not $nopass);
				$monstercheck = (($imonster and not ($iteardown or ($itype =~ /^62$/) or ($itype =~ /^101$/))) or not $monster);
				$alivecheck = ($ialive or not $alive);
				$playercheck = ((($itype =~ /^37$/) or ($itype =~ /^1$/)) or (($itype =~ /^74$/) and ($istype =~ /^31$/)) or not $player);
				$bsearchcheck = (($bodyinfo =~ /$bodyre/) or not $searchbstring);
				$searchcheck = ((($name =~ /$namere/) or ($pname =~ /$namere/)) or not $searchstring);
				$racecheck = (($race =~ /$racere/) or not $racestring);
				$slaycheck = (($slay =~ /$slayre/) or not $slaystring);
				$floorcheck = ($ifloor or not $floor);
				$spellcheck = (($itype =~ /^102$/) or not $spelleffect);
				$typecheck1 = (($itype == $typenum) or not $checktype);
				$typecheck2 = (($istype == $stypenum) or not $checkstype);
				$exitcheck = (($itype =~ /^66$/) or not $exitflag);
				$telecheck = (($itype =~ /^41$/) or not $teleflag);
				$weapcheck = (($itype =~ /^15$/) or not $weapflag);
				$bowcheck = (($itype =~ /^14$/) or not $bowflag);
				$arrowcheck = (($itype =~ /^13$/) or not $arrowflag);
				$armourcheck = ((($itype =~ /^99$/) or ($itype =~ /^100$/) or ($itype =~ /^104$/) or ($itype =~ /^87$/) or ($itype =~ /^113$/) or ($itype =~ /^34$/) or ($itype =~ /^16$/) or ($itype =~ /^133$/)) or not $armourflag);
				$currcheck = ((($itype =~ /^36$/) or ($itype =~ /^60$/)) or not $currencyflag);
				$criteria = ($nopasscheck and $monstercheck and $alivecheck and $playercheck and $searchcheck and $bsearchcheck and $racecheck and $slaycheck and $floorcheck and $spellcheck and $typecheck1 and $typecheck2 and $exitcheck and $telecheck and $weapcheck and $bowcheck and $arrowcheck and $armourcheck and $currcheck);
			}
			if ($criteria) {
				#split the face list into an array and go through the elements
				@facearray = split(/\n/, $faces);
				foreach $face2(@facearray) {
					$veryoldface=$face2;
					if ($imagefileflag or $imagemagickflag) {$face2 =~ s/\./\.base\./; $face2=$face2.".png";}
					$unique{$face2}++;
					#check that we haven't already outputted this face
					if ((($unique{$face2}<=1) or $duplicateflag) and not (($face2 =~ /^blank.111$/) or ($face2 =~ /^empty.111$/))) {
						$oldface = $face2;
						if ($showobjectflag) { $face2=$name.": ".$face2; }
						if ($showfileflag) { $face2=$File::Find::name.": ".$face2; }
						#make sure that we never output the EXACT same line twice
						$unique2{($File::Find::name.": ".$name.": ".$face2)}++;
						if (($unique2{($File::Find::name.": ".$name.": ".$face2)}<=1) and (($stdinlist{$oldface}>=1) or not ($stdinlimit=~/limit/))) {
							if (!$imagemagickflag) {print $face2."\n";}
							$fcount++;
							if ($imagelabelsflag) {$imprams="-label ".$veryoldface." ";} else {$imprams="";}
							$outfacefiles[$fcount] = $imprams.$pngloc{$oldface};
						}
					}
				}
			}

			$morestatus=0;
			$doneobject=0;
		}
	}
	close (ARCHDATA);
}
}

sub facehandle {
if (/^.*\.face\z/s) {
	open (FACEDATA, $File::Find::name) || die "Couldn't open $File::Find::name";
	while ($record = <FACEDATA>)	{
		@words = split(/ /,$record);
		if ($record =~ /^mina/) {
			$countframes=0;
			$animfaces{$aname} = $afaces;
			$afaces="";
			$aname="";
		}
		if (($countframes==1) && ($record =~ /^facings /)) {next;}
		if ($countframes==1) { $record =~ s/\s+$//; $afaces = $afaces.$record."\n"; next; }
		if ($record =~ /^animation /) { $countframes=1; $words[1] =~ s/\s+$//; $aname=$words[1]; next; }
	}
	close(FACEDATA);
} elsif (/^.*\.png\z/s) {
	#record exact locations of png images
	$tmpname = $File::Find::name;
	$tmpdir = $File::Find::dir."/";
	$tmpname =~ s/$tmpdir//;
	$tmpname =~ s/\s+$//;
	$pngloc{$tmpname} = $File::Find::name;
}
}

sub imagemagickdisp {
	$ifiles = "";
	foreach $ifloc(@outfacefiles) {
		$ifloc =~ s/\s+$//;
		$ifiles=$ifiles.$ifloc." ";
	}
	if ($imagelabelsflag) {$geometry="110x80+0+0>";} else {$geometry="70x70+0+0>";}
	$montagecmd = "montage -geometry ".$geometry." -frame 2x2+0+0 -tile 8 -background grey -mattecolor grey ";
	$montagefile = "/tmp/cffaces.png";
	$viewcommand = "eog ";
	print "Face list assembled.\n";
	#print "Command:\n".$montagecmd.$ifiles.$montagefile."\n";
	system(split(/ /, ($montagecmd.$ifiles.$montagefile) ));
	print "Face images read. Displaying faces...\n";
	exec(split(/ /, ($viewcommand.$montagefile) ));
}

