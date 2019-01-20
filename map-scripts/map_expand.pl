#! /usr/bin/perl

# this script takes a map (in new format, eg those that support
# tiling and only save the head for multipart objects) and
# expands it by some factor.  Note that editing of the destination
# file will certainly be necessary, but this may be useful instead
# of having to re-do a scaled map by hand.

$default_X_size = 16;
$default_Y_size = 16;

$expand = 2;
$help = 0;
$input_map = $ARGV[$#ARGV];

# argv loop
foreach $i (0 .. $#ARGV) {
  if($ARGV[$i] =~ "-h") { $help = 1; }
  if($ARGV[$i] =~ "-e") { $expand = $ARGV[++$i]; }
}

# various help/runtime messages
if(!$expand||!$input_map) {
  print "USAGE: $0 -e factor <input map> > <output map> \n" ;
  exit 0;
}
if($help) {
  print "\n$0 options:\n" ;
  print "-e\t Factor by which to expand map x,y dimensions.\n";
  print "-h\t This help message. \n";
  exit 0;
}

#Read in input map
open(FILE, $input_map)  || die "FATAL: file $input_map not found!\n";
# process the map object special.  This is easier than trying
# to handle the special values it has

while (<FILE>) {

    if (/^width (\d+)$/) {
	printf "width %d\n", $1 * $expand;
    } elsif (/^height (\d+)$/) {
	printf "height %d\n", $1 * $expand;
    } elsif (/^enter_x (\d+)$/) {
	printf "enter_x %d\n", $1 * $expand;
    } elsif (/^enter_y (\d+)$/) {
	printf "enter_y %d\n", $1 * $expand;
    }
    else { print $_; }
    last if (/^end$/);
}
@mapdata=<FILE>;
close(FILE);


# convert map data into objects
while ($i<=$#mapdata) {
  local(@datum) = split (' ',$mapdata[$i]);
  if($datum[0] eq "arch") { $name[$objnum] = $datum[1]; }
  elsif($datum[0] eq "end") { $objnum++; }
  elsif($datum[0] eq "x") { $x[$objnum] = $datum[1]; } 
  elsif($datum[0] eq "y") { $y[$objnum] = $datum[1]; } 
  else {
    push(@otherline,$mapdata[$i]); $olines_in_obj[$objnum]++; 
  }
  $i++; 
}


#Expand the map objects 1 to $objnum 
for ($j=0; $j<$objnum; $j++) {
  &expand_obj("$j $expand $bufline");
  $bufline += $olines_in_obj[$j];
}

# SUBROUTINES

sub expand_obj {
  local($data) = @_;
  local(@temp) = split(' ',$data);
  local($obj) = $temp[0];
  local($factor) = $temp[1];
  local($end_buf) = $temp[2] + $olines_in_obj[$obj];
  local($start_x) = $x[$obj] * $factor;
  local($start_y) = $y[$obj] * $factor;
  local($end_x) = $start_x + $factor;
  local($end_y) = $start_y + $factor;

  while($start_x<$end_x) {
    while($start_y<$end_y) { 
        local($start_buf) = $temp[2];
        if($name[$obj]) { printf("arch %s\n",$name[$obj]); } 
        else { return; }

	printf("x %d\n",$start_x);
	printf("y %d\n",$start_y);

        while ($start_buf<$end_buf) {
          print "$otherline[$start_buf]"; 
          $start_buf++;
        }
        print"end\n";
        $start_y++;
    }
    $start_x++;
    $start_y = $y[$obj] * $factor;
  }
}
