#!/usr/bin/perl

# basic

$in = $ARGV[0];
$out = $ARGV[1];
sub die {
    $prog = $0;
    print STDERR $prog.": ".$_[0]."\n";
    exit(1);
}

open(FOUT,">".$out) || &die("cannot open ".$out);


sub include_file {
    $in = shift;
    local(*FIN);
    open(FIN,"<".$in) || &die("cannot open ".$in);
    while (<FIN>) {
	if (/^<!--#include file="(.*)"-->$/) {
		&include_file($1);
	}
	else {
		print FOUT $_;
	}
    }
    close(FIN);
}

&include_file($in);
close(FOUT);
