#!/usr/bin/perl

# by peterm@langmuir.eecs.berkeley.edu

#  This program takes a .xpm as input and splits into 
#  24x24 chunks.

$base_name=$ARGV[0];
print "splitting ", $base_name, "\n";

open(XPM_FILE,$ARGV[0]) || die "Usage: splitter.pl <filename>";

# get the header stuff from the original file.
while ( (@tmp=split(/[ \s]+/,$headline=<XPM_FILE>))[0] eq "/*") {
};
@name=split(/\./,$ARGV[0]);
#$name1 = (@tmp=split(/\*/,$headline))[1];
#$name2 = (@tmp=split(/[=\[\{]/,$name1))[0];


print "name of the bitmap is:", $name[0], "\n";
#get the dimensions/parameters of the original xpm file.

while ( (@line=split(/[ \s"]+/,<XPM_FILE>))[0] eq "/*") {
};

$height = $line[2];
$ncolors = $line[3];
$width = $line[1];
$chars_per_pixel= $line[4];

print "width = ",$width, " ", "height =", $height, "  number colors =", $ncolors, " chars per pixel =", $chars_per_pixel, "\n";

if ( $height % 24 != 0 || $width % 24 != 0 ) {
	die "Use some other program to make the dimensions of this xpm divisible by 24 first.\n";
}

$i_color=0;  # index for getting lines with colors
while($i_color < $ncolors) {
  $tmp= <XPM_FILE>;  #get one line from the xpm file
  if(substr($tmp,0,1) eq '"') {
    @colors[$i_color] = $tmp;
    $i_color++;
  }

}

#read in the rest of the bitmap
$n=0;
while( $n < $height ) {
  $tmp = <XPM_FILE>;
  if(substr($tmp,0,1) eq '"') {
    @picture[$n] = $tmp;
    $n++;
  }
}

#@picture = <XPM_FILE>;

$n_rows = $height / 24;
$n_cols = $width / 24;

# maximum supported dimension has 61 squares
$indexes = "123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

#  Now the actual work of splitting the xpm file into many little files.

print $n_rows," ",$n_cols,"\n";
for( $j = 0; $j < $n_rows; $j++) {
  for( $i = 0; $i < $n_cols; $i++ ) {
    #choose a filename
    $outname = join('',$name[0],"_",substr($indexes,$i + $j*$n_cols,1));
    $out_fname = join('',$name[0],".",substr($indexes,$i + $j*$n_cols,1),"11.xpm");
    print $outname," ",$out_fname,"\n";
    $outname2= join('',$outname,"_xpm");

    #open the dest file
    open(OUTFILE,">$out_fname");

    # Write the header lines
    print OUTFILE "/* XPM */\n";
    print OUTFILE join(' ',"static","char","*",$outname2, "[] = {\n");
    # Write the dimension line
     print OUTFILE join(' ','"',"24","24",$ncolors,$chars_per_pixel,'"',",\n");

    # Write the colors
    for($i_color=0;$i_color < $ncolors; $i_color++) {
	 print OUTFILE $colors[$i_color];
    };

   # Write the proper bitmaps!
   for($k=0;$k<23;$k++) {
        print OUTFILE join('','"',substr($picture[$j*24+$k],$i*24+1,24*$chars_per_pixel),'"',",\n");
    }
   print OUTFILE join('','"',substr($picture[$j*24+$k],$i*24+1,24*$chars_per_pixel),'"',"};\n");

   }
}
