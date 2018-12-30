#!/usr/bin/perl
eval 'exec /usr/bin/perl -S $0 ${1+"$@"}'
    if $running_under_some_shell;
			# this emulates #! processing on NIH machines.
			# (remove #! line above if indigestible)

eval '$'.$1.'$2;' while $ARGV[0] =~ /^([A-Za-z_0-9]+=)(.*)/ && shift;
			# process any FOO=bar switches

# items-extract - parse the archetypes-file and output the
# artifacts in a structured format.

# Variables passed when invoked:
#   living_c - filename where the array attacks is defined.

$[ = 1;			# set array base to 1

# These stats will be added to the "magik" string according
# to the pattern. "%s" should be "%+d", but that isn't
# portable.
$magic{'Str'} = 'strength %s';
$magic{'Dex'} = 'dexterity %s';
$magic{'Con'} = 'constitution %s';
$magic{'Int'} = 'intelligence %s';
$magic{'Wis'} = 'wisdom %s';
$magic{'Cha'} = 'charisma %s';
$magic{'Pow'} = 'power %s';

$magic{'luck'} = 'luck %s';
$magic{'exp'} = 'speed %s';
$magic{'sp'} = 'spell-point regeneration %s';
$magic{'hp'} = 'hit-point regeneration %s';
#	magic["dam"] = "damage %s";

$magic{'reflect_spell'} = 'reflect spells';
$magic{'xrays'} = 'X-ray vision';
$magic{'stealth'} = 'stealth';
$magic{'flying'} = 'flying';

# Read the attack-types (and immune/protection)
while ((($buff = &Getline3($living_c),$getline_ok)) == 1) {
    if ($buff =~ /attacks\[/) {
	$att = 0;
	while (1) {
	    $buff = &Getline3($living_c);
	    if ($buff =~ '^}') {
		last;
	    }
	    $s = "[ \t]*\"", $buff =~ s/$s//g;
	    $nr = (@arr = split(/,/, $buff, 9999));
	    for ($i = 1; $i <= $nr && $arr[$i]; $i++) {
		$attack{++$att} = $arr[$i];
	    }
	}
	last;
    }
}
delete $opened{$living_c} && close($living_c);

# These types are always artifacts:
$artifact{99} = $artifact{14} = $artifact{16} = $artifact{33} = 1;
$artifact{34} = $artifact{100} = $artifact{113} = $artifact{915} = 1;

$weapons{15} = $weapons{915} = 1;
$armours{16} = $armours{33} = $armours{34} = $armours{99} = 1;

$worthless{'chair'} = $worthless{'table'} = $worthless{'bed'} = 1;

while (<>) {
    chomp;	# strip record separator
    @Fld = split(' ', $_);
    if (/^Object (.*)/) {
	$slay = $magik = '';
	$name = $obj = $1;
	$xmin = $xmax = $ymin = $ymax = 0;
	$More = 0;
	$att = $dam = $type = $magical = $ac = $armour = $weight = $last_sp = 0;
	$prot = "";
    }

    if (defined $magic{$Fld[1]}) {
	if ($Fld[1] eq 'sp' && $type == 14) {
	    $ac = $Fld[2];
	}
	else {
	    &add_magik($magic{$Fld[1]}, $Fld[2]);
	}
    }

    if (/^type/) {
	$type = $Fld[2];
    }
    if (/^last_sp/) {
	$last_sp = $Fld[2];
    }
    if (/^dam/) {
	$dam = $Fld[2];
    }
    if (/^ac/) {
	$ac = $Fld[2];
    }
    if (/^armour/) {
	$armour = $Fld[2];
    }
    if (/^resist_physical/) {
	$armour = $Fld[2];
    }
    if (/^weight/) {
	$weight = $Fld[2];
    }
    if (/^attacktype/) {
	$att = $Fld[2];
    }
    if (/^immune/) {
	$immune = $Fld[2];
    }
    if (/^vulnerable/) {
	$vulnerable = $Fld[2];
    }
    if (/^slaying/) {
	$slay = $Fld[2];
    }
    if (/^magic/) {
	$magical = $Fld[2];
    }
    if (/^name /) {
	$name = substr($_, 6, 999999);
    }
    if (/^resist_([a-z]+) (-*\d+)/) {
	if ($1 ne "physical") {
	    if ($2 > 0) { $n = "+$2"; } else {$n = "$2"; }
	    if ($prot eq "") {
		$prot = "$1 $n";
	    } else {
		$prot .= ", $1 $n";
	    }
	}
    }

    if (/^end/) {
	# Type 15 are artifacts if they are magical
	if ($type == 15 && $magical) {
	    $type += 900;
	    # It can also be chairs and beds, but they are in the worthless
	    # array...
	    ;
	}
	if ($artifact{$type} || ($type == 15 && !$worthless{$name})) {
	    if ($dam && !(defined $weapons{$type})) {
		&add_magik('damage %s', $dam);
	    }
	    if ($ac && !(defined $armours{$type})) {
		&add_magik('ac %s', $ac);
	    }
	    if ($armour && !(defined $armours{$type})) {
		&add_magik('armour %s', $armour);
	    }
	    $magik = $magik . &attacktype($att, 'Attacks:');
	    $magik = $magik . "<br>Protections: $prot" if ($prot ne "");
	    if ($slay eq "wall")  {
		$magik = $magik . "<br>Excavation";
	    } elsif ($slay ne "" ) {
		$magik = $magik . "<br> ".  &capitalize("$slay" . "-slaying");
	    }

	    if ($magical) {
		$name = $name . ' +' . $magical;
	    }
	    $s = '^<br> ', $magik =~ s/$s//;
	    $magik = &capitalize($magik);
	    $name = &capitalize($name);
	    $s = '_', $name =~ s/$s/ /;

	    if (defined $armours{$type}) {
		$speed = $last_sp / 10;
	    }
	    elsif (defined $weapons{$type}) {
		# Horrible, I know. Blame vidarl@ifi.uio.no -- Fy Vidar!
		# I assume the player has max Str and Dex
		# and speed of 6 here.

		# weapon_speed = (last_sp*2 - magical) / 2;
		# if (weapon_speed < 0) weapon_speed = 0;

		# M = (300-121)/121.0;
		# M2 = 300/100.0;
		# W = weight/20000.0;
		# s = 2 - weapon_speed/10.0;

		# D = (30-14)/14.0;
		# K = 1 + M/3.0 - W/(3*M2) + 6/5.0 + D/2.0;
		# K *= (4 + 99)/(6 + 99) * 1.2;
		# if ( K <= 0) K = 0.01

		# W = weight/20000; s = 2 - ((last_sp*2 - magical) / 2)/10;
		# K = 1.177*(4 - W/30 + 6/5)
		# if (K <= 0) K = 0.01;

		# speed = 6/(K*s);

		$speed = $last_sp;
	    }
	    else {
		$speed = 0;
	    }
	    printf "%d &%s &%s &%s &%d &%.1f &%d &%d &%d &~~%s~~ &%.2f\n",
	    $type, $obj, $name, $magik, $dam, ($weight / 1000), $ac,
	    $armour, $magical, $obj, $speed;
	}
    }

    # Given a bitmask, give a string enumerating the meaning of the bits.
}

delete $opened{'items'} && close('items');

sub attacktype {
    local($at, $type, $i, $str) = @_;
    for ($i = 1; defined $attack{$i}; $i++) {
	if ($at % 2) {
	    $str = ($str ? $str . ', ' : '') . $attack{$i};
	}
	$at = int($at / 2);
    }
    ($str ? '<br> ' . $type . ' ' . $str : '');
}

sub add_magik {
    local($str, $val) = @_;

    if ($str =~ /%[0-9-]*s/) {
          $str = sprintf($str, $val < 0 ? $val : "+". $val);
    }
    $magik = $magik ? $magik . ', ' . $str : $str;
}

sub capitalize {
    local($str) = @_;
    $a = substr($str, 1, 1);
    $a =~ tr/a-z/A-Z/;
    $_ = $a .  substr($str, 2, 999999);

}

sub Getline3 {
    &Pick('',@_);
    local($_);
    if ($getline_ok = (($_ = <$fh>) ne '')) {
	chomp;	# strip record separator
    }
    $_;
}

sub Pick {
    local($mode,$name,$pipe) = @_;
    $fh = $name;
    open($name,$mode.$name.$pipe) unless $opened{$name}++;
}
