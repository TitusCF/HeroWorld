#!/bin/sh
#
#    CrossFire, A Multiplayer game for X-windows
#    Copyright (C) 2007 Nicolas Weeger
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#    The authors can be reached via e-mail at crossfire-devel@real-time.com
#
# This script creates a plugin from the template directory.
# The plugin will have for id (slaying field in events) the first argument,
# and for name an optional second argument, or the id.
#
# Running multiple times this script will erase generated/modified
# plugin, but should not mess too much with building process.
#
# Note that some Crossfire files are affected.
# Ensure everything works before committing :)
#
# Also, modified files (/configure.ac) may have extra new lines.

if test  "x$1" = "x"  ; then
	echo "This script creates a plugin from the template directory."
	echo "Usage: $0 plugin_id [plugin_name]"
	exit
fi

ID=$1
NAME=$ID
if test "x$2" != "x" ; then
	NAME=$2
fi

echo "Creating plugin $NAME ($ID)"

DIR=$1
FIC_INC=$DIR/include/$ID.h
FIC_C=$DIR/$ID.c
FIC_MAK=$DIR/Makefile.am

echo "Copying files..."
cd ..
mkdir $1
cp template/plugin_template.c $FIC_C
mkdir $1/include
cp template/include/plugin_template.h $FIC_INC
cp template/Makefile.am $FIC_MAK

echo "Altering plugin's include"
sed -i -e "s/PLUGIN_NAME\s\+\"Template\"/PLUGIN_NAME \"$ID\"/g" $FIC_INC
sed -i -e "s/PLUGIN_VERSION\s\+\"Template Plugin 2.0\"/PLUGIN_VERSION \"$NAME plugin version 1.0\"/" $FIC_INC
sed -i -e "s/PLUGIN_TEMPLATE_H/PLUGIN_$ID\_H/" $FIC_INC
sed -i -e "s/plugin_template.h/$ID.h/" $FIC_INC
sed -i -e "s/plugin_template_proto.h/${ID}_proto.h/" $FIC_INC

echo "Altering plugin's c"
sed -i -e "s/plugin_template.h/$ID.h/" $FIC_C

echo "Altering plugin's Makefile.am"
sed -i -e "s/plugin_template/$ID/g" $FIC_MAK

PLUGINS=Makefile.am
echo "Adding plugin to build list in /plugins/Makefile.am"
sed -i -e "s/ $ID / /" $PLUGINS
sed -i -e "s/SUBDIRS =/SUBDIRS = $ID/" $PLUGINS

echo "Adding plugin to configure.ac"
CONF=../configure.ac
sed -i -e "s/plugins\/$ID\/Makefile/ /" $CONF
sed -i -e "s/devel\/Makefile/plugins\/$ID\/Makefile\n\tdevel\/Makefile/" $CONF

echo "Running auto* tools"
( cd .. && automake && autoconf )

echo
echo
echo "Done. Please run configure in the top-level directory to correctly generate Makefiles."
