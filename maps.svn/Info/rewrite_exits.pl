#!/usr/bin/perl
# this will adjust the 'slaying' lines
# in the current directory, and all sub-directories,
# so shuffling maps to new directories is not a problem.
#
# This absolutely depends on each map having a unique name.
# This script should be placed outside of the map directory,
# but you should be inside it when you run it.  Ideally
# in the top-level maps directory.
# 
# Dupes are noted, and not adjusted.
# 

use File::Basename qw( basename );
use vars qw( $MAPBASE );
use strict;
use warnings;

# this is the base directory of the maps.
# The slaying linesthatpointtoothermaps,
# will be re-written as $MAPBASE/filename
$MAPBASE = "/mlab";

my %FILES = ();
my %DUPES = ();
open(FIND, "find ./|");
while (<FIND>) {
	chomp;
	next unless -f;
	next if $_ =~ /editor/;
	my $basename = basename($_);
	if ($FILES{$basename}) {
		unless (exists $DUPES{$basename}) {
			$DUPES{$basename} = [];
			push(@{ $DUPES{$basename} }, $FILES{$basename});
		}
		push(@{ $DUPES{$basename} }, $_);
		#warn "DUPE!: $_ => $FILES{$basename}\n";
	}
	else {
		$FILES{$basename} = $_;
	}
}
close(FIND);
foreach my $file (keys %DUPES) {
	delete $FILES{$file};
	print "Dupe filename: $file Sizes:";
	foreach my $x (@{ $DUPES{$file} }) {
		printf " %s", -s $x; # file size
		printf " %s", $x; # file path
	}
	print "\n";
}

foreach my $file (keys %FILES) {
	open(FILE, "<$FILES{$file}");
	my $data = join("", <FILE>);
	close(FILE);

	my $newdata = $data;

	$newdata =~ s/(slaying|final_map)\s+(.+)/mappath($1,$2)/eg;
	if ($newdata ne $data) {
		open(FILE, ">$FILES{$file}");
		print FILE $newdata;
		close(FILE);
	}
	#printf("%s => %s\n", $file, $FILES{$file});
}

sub mappath {
	my ($type,$name) = @_;
	$name =~ s/\s+//g;
	my $file = basename($name);
	my $new = $FILES{$file};
	if ($new) {
		unless ($file =~ /[a-z]/) {
			print "CASE?: $file\n";
		}
		$new =~ s/^\.\///;
		return "$type $MAPBASE/$new";
	}
	return "$type $name";
}
