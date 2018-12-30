SUBDIRS = common random_maps socket server

CHECK_ROOT=../
CHECK_FOLDER=unit
CHECK_PARENT_NAME=Main

include $(top_srcdir)/check-fragment.am

DISTCLEANFILES=	.autorun.xml

check-local: check-html
clean-local: clean-unit
