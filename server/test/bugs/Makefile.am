CROSSEDIT = @CROSSEDIT@

SUBDIRS = bugtrack

CHECK_ROOT=../
CHECK_FOLDER=bugs
CHECK_PARENT_NAME=Main

include $(top_srcdir)/check-fragment.am

DISTCLEANFILES=	.autorun.xml

check-local: check-html
clean-local: clean-unit
