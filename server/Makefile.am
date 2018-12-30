XSLTENGINE= @XSLTENGINE@

if HAVE_CHECK
  UNIT_TEST = test
endif

SUBDIRS = include common random_maps socket types server lib utils doc plugins $(UNIT_TEST)

EXTRA_DIST = AUTHORS ChangeLog COPYING DEVELOPERS autogen.sh run_win32.bat

DISTCHECK_CONFIGURE_FLAGS = --localstatedir=$$dc_install_base/var --mandir=$$dc_install_base/man

ACLOCAL_AMFLAGS = -I macros --install

archives: dist
	(cd doc; make archive)
	(cd lib; make archive)

proto:
	(cd common; make proto)
	(cd server; make proto)
	(cd plugins; make proto)
	(cd types; make proto)

CLEANFILES=	configtest.out configtest.xml configtest.xsl
