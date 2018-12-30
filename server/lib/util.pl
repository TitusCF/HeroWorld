######################################################################
# subs
######################################################################

###
sub capitalize {
    local($str) = $_[0];
    local($head) = ord(substr($str,0,1));
    local($tail) = substr($str,1);
    $head = $head - 32 if $head >= 97 && $head ne '_';
    return sprintf("%c%s",$head,$tail);
}

###
sub uncapitalize {
    local($str) = $_[0];
    local($head) = ord(substr($str,0,1));
    local($tail) = substr($str,1);
    $head = $head + 32 if $head <= 97 && $head ne '_';
    return sprintf("%c%s",$head,$tail);
}

### user debug message
sub msg {
    print STDERR $0.": ".$_[0]."\n" if $debug;
}

###
sub die {
    $prog = &basename($0);
    print STDERR $prog.": ".$_[0]."\n";
    exit(1);
}

###
sub warn {
    $prog = &basename($0);
    print STDERR $prog.": ".$_[0]."\n" if ! $nowarn;
}

###
sub info {
    $prog = &basename($0);
    print STDERR $prog.": ".$_[0]."\n";
}

### basename of file
sub basename {
        local($name) = shift;
        local($ext) = shift;
	if($name =~ /.*\/.*/) {
	    $name =~ s/.*\/(.*)$ext$/$1/;
	} else {
	    $name =~ s/(.*)$ext$/$1/;
	}
        return $name;
}

###
sub dirname {
    local($name) = shift;
    $name =~ s/(^.*)\/.*$/$1/;
    return $name;
}

### make uniq to array
sub uniq {
    local(@list) = sort(@_);
    local($item,$prev);
    local(@uniq);
    foreach $item (@list) {
	push(@uniq,$item) if($item ne $prev);
	$prev = $item;
    }
    return @uniq;
}

1;

### end of util.pl ###
