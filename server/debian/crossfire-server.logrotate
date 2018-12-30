/var/log/crossfire/logfile {
	weekly
	missingok
	notifempty
	compress
	nocreate
	postrotate
		/bin/kill -SIGUSR1 `cat /var/run/crossfire.pid` || true
	endscript
}
