SUBDIRS = cfrhg citylife cfpython cfanim common cflogger cfnewspaper

DISTCHECK_CONFIGURE_FLAGS = --localstatedir=$$dc_install_base/var --mandir=$$dc_install_base/man

proto:
	(cd cfpython; make proto; cd ..; cd cfanim; make proto)
