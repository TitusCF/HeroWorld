#!/usr/bin/perl
# $Id: metaserver.pl.in 6717 2007-06-27 20:07:36Z akirschbaum $

# Copyright 2000 by Mark Wedel.
# This script follows the same license as crossfire (GPL).

use Socket;
use English;

# We periodically generate a nice HTML file that people can't put their
# web browser at.  This is the location of that file.
$HTML_FILE="/var/apache/htdocs/metaserver.html";

# Cache file to keep data we ahve collected.  This is used so that if
# the metaserver program crashes/dies, it still has some old data.
# You may want to set this to a location that will survive across system
# reboots.
$CACHE_FILE="/var/tmp/meta_xfire.cache";

# We remove a server after $REMOVE_SERVER number of seconds of no updates.
# 600 is 10 minutes - if we haven't gotten an update that fast, the server
# is almost certainly gone/not available.  This reduces congestion when
# someone on a dhcp connection keeps running a server and it fills
# up a bunch of the slots.
$REMOVE_SERVER=600;

# UPDATE_SYNC determines how often we update the HTML_FILE and CACHE_FILE.
$UPDATE_SYNC=300;

# IP_INTERVAL is how often (in seconds) same IP can request metaserver
# info.  This is to prevent DOS attacks.

$IP_INTERVAL=5;

# For gathering some anonymous statistics. You probably want to use
# MRTG/RRDTOOL for generating statistics from the file.
# -- Heikki Hokkanen, 2005-03-26
my $STATS_FILE="/var/tmp/meta_xfire.stats";
my $stats_updatehits = 0; # COUNTER
my $stats_requesthits = 0; # COUNTER
my $stats_totalplayers = 0; # GAUGE
my $stats_totalservers = 0; # GAUGE

socket(SOCKET, PF_INET, SOCK_DGRAM, getprotobyname("udp")) ||
	die("$0: can not open udp socket: $OS_ERROR\n");
bind(SOCKET, sockaddr_in(13326, INADDR_ANY)) ||
	die("$0: Can not bind to socket: $OS_ERROR\n");

# Socket1 is used for incoming requests - if we get a connection on this,
# we dump out our data to that socket in an easily parsible form.
socket(SOCKET1, PF_INET, SOCK_STREAM, getprotobyname("tcp")) ||
	die("$0: can not open tcp socket: $OS_ERROR\n");

# errors on this not that critical
setsockopt(SOCKET1, SOL_SOCKET, SO_REUSEADDR, 1);

bind(SOCKET1, sockaddr_in(13326, INADDR_ANY)) ||
	die("$0: Can not bind to socket: $OS_ERROR\n");
listen(SOCKET1, 10) ||
	die("$0: Can not listen on socket: $OS_ERROR\n");

vec($rin, fileno(SOCKET), 1)=1;
vec($rin, fileno(SOCKET1), 1)=1;

if (open(CACHE,"<$CACHE_FILE")) {
    while (<CACHE>) {
	chomp;
	($ip, $rest) = split /\|/, $_, 2;
	$data{$ip} = $_;
    }
}
close(CACHE);

$last_sync=time;

while (1) {
    $nfound=select($rout=$rin, undef, undef, 60);
    $cur_time=time;
    if ($nfound) {
	if (vec($rout, fileno(SOCKET),1)) {
	    $ipaddr = recv(SOCKET, $data, 256, 0) ||
		print STDERR "$0: error on recv call: $OS_ERROR\n";
	    ($port, $ip) = sockaddr_in($ipaddr);
	    $host = inet_ntoa $ip;
	    ($name, $rest) = split /\|/, $data;
	    if ($name ne "put.your.hostname.here") {
		$data{$host} = "$host|$cur_time|$data";
		$stats_updatehits++;
	    }
	}
	if (vec($rout, fileno(SOCKET1),1)) {
	    # This is overly basic - if there are enough servers
	    # where the output won't fit in the outgoing socket,
	    # this will block.  However, if we fork, we open
	    # ourselves up to someone intentionally designing something
	    # that causes these to block, and then have us fork a huge
	    # number of process potentially filling up our proc table.
	    if ($ipaddr=accept NEWSOCKET, SOCKET1) {
		($port, $ip ) = sockaddr_in( $ipaddr );
		$dq = join('.',unpack('C4', $ip));
		if ($ip_times{$dq} > time) {
		    close(NEWSOCKET);
 		} else {
		    $ip_times{$dq} = time + $IP_INTERVAL;
		    foreach $i (keys %data) {
		        # Report idle time to client, and not last update
		    	# as we received in seconds since epoch.
			($ip, $time, $rest) = split /\|/, $data{$i}, 3;
		    	$newtime = $cur_time - $time;
		    	print NEWSOCKET "$ip|$newtime|$rest\n";
		    }
		    close(NEWSOCKET);
		    $stats_requesthits++;
		}
	    }
	}
    }

    # Need to generate some files.  This is also where we remove outdated
    # hosts.
    if ($last_sync+$UPDATE_SYNC < $cur_time) {
	$last_sync = $cur_time;
	open(CACHE,">$CACHE_FILE");
	open(HTML,">$HTML_FILE");

	print HTML
'<title>Crossfire Server List</title>
<h1 align=center>Crossfire Server List</h1><p>
<table border=1 align=center cellpadding=5>
<tr>
<th>IP Address</th><th>Last Update Date/Time</th><th>Last Update Minutes Elapsed</th>
<th>Hostname</th><th>Number of Players</th><th>Version</th><th>Comment</th>
</tr>
';

	$stats_totalplayers = 0;
	$stats_totalservers = 0;
	foreach $i (keys %data) {
		$stats_totalservers++;
	    ($ip, $time, @rest) = split /\|/, $data{$i};
	    if ($time+$REMOVE_SERVER<$cur_time) {
		delete $data{$i};
	    } else {
		print CACHE "$data{$i}\n";
		$elapsed = int(($cur_time - $time)/60);
		$gmtime = gmtime($time);
		print HTML "<tr><td>$i</td><td>$gmtime</td><td>$elapsed</td>";
		print HTML "<td>$rest[0]</td><td>$rest[1]</td><td>$rest[2]</td><td>$rest[3]</td></tr>\n";
		$stats_totalplayers += int($rest[2]);
	    }
	}
	$gmtime = gmtime($cur_time);
	print HTML "
</table><p>
The server name is reported by the server, while the ip address is determined by
the incoming data packet.  These values may not resolve to the same thing in the
case of multi homed hosts or multi ip hosts.<p>

All times are in GMT.<p>

<font size=-2>Last Updated: $gmtime<font size=+2><p>";
	close(HTML);
	close(CACHE);

	open(STATS,">$STATS_FILE");
	print STATS "$stats_updatehits:$stats_requesthits:$stats_totalservers:$stats_totalplayers\n";
	close(STATS);

    }
}
