#!/usr/bin/perl
eval 'exec /usr/bin/perl -S $0 ${1+"$@"}'
    if $running_under_some_shell;
			# this emulates #! processing on NIH machines.
			# (remove #! line above if indigestible)

eval '$'.$1.'$2;' while $ARGV[0] =~ /^([A-Za-z_0-9]+=)(.*)/ && shift;
			# process any FOO=bar switches

$[ = 1;			# set array base to 1


# special is a list of what special things we should look for.
# The value of the array is how many commas we should skip.
$special{'Attacks'} = 1;
$special{'Spell abilities'} = 1;

while (<>) {
    chomp;
    ($Fld1,$Fld2,$Fld3,$Fld4,$Fld5,$Fld6,$Fld7) = split('\|');
    # Expl:
    # name 		- ..
    # comma 	- Print a comma or not
    # antall 	- number of (sub)fields in the 'Special' field; antall(Nor) <-> "number of".
    # i	 	- counter. Should start as values 2.

    $name = &capitalize($Fld1);
    $s = '_', $name =~ s/$s/ /;
    $comma = 0;
    $first_resist=0;
    # Split on the parens
    @field = split('[():]', $Fld5, 9999);

    printf
      '<tr><th>%s</th><td>~~%s~~</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>',
	    $name, $Fld6, $Fld7 ? '~~' . $Fld7 . '~~' : '', $Fld2, $Fld3, $Fld4;

    for ($i = 1; $i < $#field; $i++) {
	next if ($field[$i] eq "" );

	if (defined $special{$field[$i]}) {
	    printf '<br>%s: ', &capitalize($field[$i]);
	    $comma =0;
	}
	elsif ($first_resist == 0 && $field[$i] =~/^(resist|armour)/) {
	    # We want to put the first reist value on its own line, and
	    # capitalize it.
	    printf '<br>%s', &capitalize($field[$i]);
	    $first_resist=1;
	    $comma=1;
	} else {
	    if ($comma > 0) {
		print ", $field[$i]";
	    } else {
		print &capitalize($field[$i]);
	        $comma = 1;
	    }
	}
    }
    print "</td></tr>\n";
}


sub capitalize {
    local($str) = @_;
    $a = substr($str, 1, 1);
    $a =~ tr/a-z/A-Z/;
    $_ = $a .  substr($str, 2, 999999);

}
