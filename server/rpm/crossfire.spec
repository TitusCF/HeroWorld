%define name crossfire
%define version 0.96.0
%define release 1

Name: %{name}
Version: %{version}
Release: %{release}
Summary:  Role-playing graphical adventure game.
Group: Amusements/Games/Crossfire
Copyright: GPL
URL: http://crossfire.real-time.com
Packager: Real Time Enterprises, Inc. <bugs@real-time.com>
# This is the canonical source:
Source0: ftp://ftp.scruz.net/users/mwedel/public/%{name}-%{version}.tar.gz
Source1: ftp://ftp.scruz.net/users/mwedel/public/%{name}-%{version}.maps.tar.gz
Source2: ftp://ftp.scruz.net/users/mwedel/public/%{name}-%{version}.arch.tar.gz
Source3: crossfire.init
Source4: crossfire.logrotate
Prefix: /usr/games/crossfire
BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root
Requires: crossfire-common crossfire-maps

%description
Crossfire is a highly graphical role-playing adventure game with
characteristics reminiscent of rogue, nethack, omega, and gauntlet.  It has
multiplayer capability and presently runs under X11.

#
# crossfire-common
#
%package common
Group: Amusements/Games/Crossfire
Summary: Common files for all part of crossfire.

%description common
This package contains the files that are shared between crossfire-devel,
crossfire-maps and the crossfire server pages.

#
# crossfire-devel
#
%package devel
Group: Amusements/Games/Crossfire
Summary: Zone building tools for crossfire
Requires: crossfire-common crossfire-maps

%description devel
Tools and files needed to build zone for crossfire.

#
# crossfire-maps
#
%package maps
Group: Amusements/Games/Crossfire
Summary: Crossfire maps
Requires: crossfire-common

%description maps
Zone files necessary for crossfire.

#
# crossfire-doc
#
%package doc
Group: Amusements/Games/Crossfire
Summary: Crossfire documentation.

%description doc
Crossfire documentation.

%prep
%setup

%build
chmod 755 configure
CFLAGS="$RPM_OPT_FLAGS" \
./configure --prefix=%{prefix} \
  --datadir=%{prefix}/share \
  --localstatedir=%{prefix}/var \
  --bindir=%{prefix}/bin \
  --mandir=/usr/man \
  --enable-old-layout=no

make CFLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT

install -d $RPM_BUILD_ROOT/etc/rc.d/init.d
install -d $RPM_BUILD_ROOT/etc/logrotate.d
install -d $RPM_BUILD_ROOT/usr/games/crossfire/bin
install -d $RPM_BUILD_ROOT/usr/games/crossfire/share
install -d $RPM_BUILD_ROOT/usr/games/crossfire/var/logs

make install DESTDIR="$RPM_BUILD_ROOT" \
	prefix="$RPM_BUILD_ROOT/%{prefix}" \
	datadir="$RPM_BUILD_ROOT/%{prefix}/share" \
	localdir="$RPM_BUILD_ROOT/%{prefix}/var" \
	bindir="$RPM_BUILD_ROOT/%{prefix}/bin" \
	mandir="$RPM_BUILD_ROOT/usr/man"


install -c -m 755 %{SOURCE3} $RPM_BUILD_ROOT/etc/rc.d/init.d/crossfire
install -c -m 644 %{SOURCE4} $RPM_BUILD_ROOT/etc/logrotate.d/crossfire
touch $RPM_BUILD_ROOT/usr/games/crossfire/var/logs/crossfire.log

#for i in `ls ${RPM_BUILD_DIR}/%{name}-%{version}/lib/*.pl`; do
#    install -c -m 755 $i $RPM_BUILD_ROOT/%{prefix}/bin
#done

(cd $RPM_BUILD_ROOT/%{prefix}/share; \
    gzip -dc %{SOURCE1} | tar -xf - ; \
    mv %{name}-%{version}.maps maps )

(cd $RPM_BUILD_ROOT/%{prefix}/share; \
    gzip -dc %{SOURCE2} | tar -xf - ; \
    mv %{name}-%{version}.arch arch )

find $RPM_BUILD_ROOT -name \*.orig -print0 | xargs -0 rm -f
find doc -name \*.orig -print0 | xargs -0 rm -f

(cd $RPM_BUILD_ROOT/%{prefix}/bin; strip crossedit crossfire random_map)

cd $RPM_BUILD_ROOT/%{prefix}
for a in `ls share/maps/world/connect.pl`; do

    cat $a | sed 's=/var/tmp/crossfire-0.95.8-1-root/==' > tmp.$$ ; \
	mv tmp.$$ $a
done

%clean
rm -rf $RPM_BUILD_ROOT

%post devel
ln -s /usr/games/crossfire/bin/crossedit /usr/X11R6/bin/crossedit

%preun devel
rm -f /usr/X11R6/bin/crossedit

%post
/sbin/chkconfig --add crossfire

%preun
if [ "$1" = 0 ] ; then
  if [ -f /var/lock/subsys/crossfire ]; then
    /etc/rc.d/init.d/crossfire stop
  fi
  /sbin/chkconfig --del crossfire
fi

%files common
%defattr(-,games,games,0755)
/usr/games/crossfire/share/animations
/usr/games/crossfire/share/archetypes
/usr/games/crossfire/share/artifacts
/usr/games/crossfire/share/bmaps
/usr/games/crossfire/share/bmaps.paths
/usr/games/crossfire/share/crossfire.*
/usr/games/crossfire/share/def_help
/usr/games/crossfire/share/faces
/usr/games/crossfire/share/formulae
/usr/games/crossfire/share/help
/usr/games/crossfire/share/messages
/usr/games/crossfire/share/races
/usr/games/crossfire/share/settings
/usr/games/crossfire/share/skill_params
/usr/games/crossfire/share/spell_params
/usr/games/crossfire/share/treasures

%files devel
%defattr(-,games,games,0755)
/usr/games/crossfire/bin/crossedit
/usr/games/crossfire/bin/collect.pl
/usr/games/crossfire/bin/xpmtopix.pl
%attr(0644,games,games)/usr/games/crossfire/bin/util.pl
/usr/man/man6/crossedit.6*
/usr/games/crossfire/share/arch

%files maps
%defattr(0644,games,games,0755)
/usr/games/crossfire/share/maps

%files doc
%defattr(0644,root,root,0755)
%doc doc

%files
%defattr(644,games,games,755)
%attr(-,root,root)%doc CHANGES CREDITS DEVELOPERS DONE INSTALL License README TODO
%attr(6755,games,games) /usr/games/crossfire/bin/crossfire
/usr/games/crossfire/bin/add_throw
/usr/games/crossfire/bin/crossloop
/usr/games/crossfire/bin/flushlocks
/usr/games/crossfire/bin/mktable
/usr/games/crossfire/bin/random_map
/usr/man/man6/crossfire.6*
/usr/games/crossfire/var/logs/crossfire.log

%dir /usr/games/crossfire/var/players
%dir /usr/games/crossfire/var/unique-items
%config /usr/games/crossfire/var/bookarch
%config /usr/games/crossfire/var/highscore
/usr/games/crossfire/var/temp.maps

%config /usr/games/crossfire/share/ban_file
%config /usr/games/crossfire/share/dm_file
%config /usr/games/crossfire/share/forbid
%config /usr/games/crossfire/share/motd

%attr(-,root,root) /etc/rc.d/init.d/crossfire

%changelog
* Mon Feb 12 2001 Bob Tanner <tanner@real-time.com>
- Split the one rpm into several. Crossfire the server, Crossfire-devel for crossedit
  and associate tools, Crossfire-maps for the maps and Crossfire-doc for the
  documentation.
- Had to split stuff into another package Crossfire-common for the stuff shared
  between all the other packages.
* Tue Mar 16 1999 Toshio Kuratomi <badger@prtr-13.ucsc.edu> [0.95.2-5]
- Edited the patch files to get rid of redundancies and excesses.
- Moved files around to conform to the File Hierarchy Standard 2.0
- Changed portions of the spec to use configure to set up the directory
  structure rather than having to construct it manually.
- No longer package crossloop.pl as it is outdated by the newer crossloop
  shell script.
- Moved the tmp directory to be in /var/lib/games/crossfire/temp-maps...
  This way the temp maps are all in the var directory for reuse.
  I don't think crossfire keeps any other temp files around.
- Remove crossserv as it isn't necessary to the operation of crossfire as a
  server. (Instead set crossfire setuid/gruid games.
- Edit the crossfire.init script to reflect the absence of crossserv.
- Add maps to the package.  Will not run without this (but maybe it should be
  in a subpackage.)

* Sun Jan 31 1999 Kjetil Wiekhorst Jørgensen <jorgens+rpm@pvv.org> [0.95.2-1]

- upgraded to version 0.95.2

* Fri Sep  4 1998 Kjetil Wiekhorst Jørgensen <jorgens+rpm@pvv.org> [0.95.1-1]

- upgraded to version 0.95.1
- fixed some minor bugs in the distribution
- dead is no longer final due to a bug in that code

* Fri Sep  4 1998 Kjetil Wiekhorst Jørgensen <jorgens+rpm@pvv.org> [0.94.3-1]

- upgraded to version 0.94.3
- moved some files around (static files to /usr/lib/games/crossfire and
  dynamic files to /var/lib/games/crossfire)

* Mon Jun 01 1998 Kjetil Wiekhorst Jørgensen <jorgens+rpm@pvv.org>

- upgraded to 0.94.2.
- removed the client stuff, since crossfire has become client/server based.

* Sat Apr 25 1998 Kjetil Wiekhorst Jørgensen <jorgens+rpm@pvv.org>

- upgraded to 0.94.1.

* Sat Feb 08 1998 Kjetil Wiekhorst Jørgensen <jorgens+rpm@zarhan.pvv.org>

- added the missing space between the number and the item name.
- fixed a bug which will allow the code to compile even if the ERIC_SERVER
  isn't defined.


* Sat Feb 08 1998 Kjetil Wiekhorst Jørgensen <jorgens+rpm@zarhan.pvv.org>

- version 0.94.0
- minor bugfix in server code.

* Sat Feb 08 1998 Kjetil Wiekhorst Jørgensen <jorgens+rpm@zarhan.pvv.org>

- new specfile for crossfire.
