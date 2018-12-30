#!/usr/bin/env bash
# $Id: png_recompress.sh 5270 2008-01-08 21:23:59Z anmaster $

#  Copyright (C) 2007-2008 Arvid Norlander <anmaster AT tele2 DOT se>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# This script recompressess .png files using optipng and
# advpng to get the smallest images. All recompression is
# looseless.
#
# This script needs at least bash 3.1, bash 3.x or 2.x will not work.
# (Note that it was only tested on bash 3.2.17 or later though.)
# Also this need the programs mentioned above (optipng and advpng).
#
# TODO:
#  * Better docs
#  * Check that input really are *.png
#  * If advpng doesn't make a file smaller, don't run the second
#    optipng pass on that file (if the first optipng pass was enabled).

# Check for new enough bash version
fail_old_bash() {
	echo "Sorry your bash version is too old!"
	echo "You need at least version 3.1 of bash."
	echo "Please install a newer version:"
	echo " * Either use your distro's packages."
	echo " * Or see http://www.gnu.org/software/bash/"
	exit 2
}

# Check bash version. We need at least 3.1
# Lets not use anything like =~ here because
# that may not work on old bash versions.
if [[ "${BASH_VERSINFO[0]}${BASH_VERSINFO[1]}" -lt 31 ]]; then
	fail_old_bash
fi

# Version
declare -r png_recompress_version='0.0.1'

# Useful for debugging
export PS4='+(${BASH_SOURCE}:${LINENO}): ${FUNCNAME[0]} : '

print_cmd_help() {
	echo 'png_recompress is a script to automate compressing of png images.'
	echo ''
	echo 'Usage: png_recompress [OPTION] files...'
	echo ''
	echo 'Options:'
	echo '  -h, --help              Display this help and exit.'
	echo '  -V, --version           Output version information and exit.'
	echo '  -r, --recursive         Run recursively on directories specified on command line. Will not follow symlinks.'
	echo '      --skip pass         Make png_recompress skip this pass (1, 2 or 3).'
	echo ''
	echo "Note that png_recompress can't handle short versions of options being written together like"
	echo "-vv currently."
	echo ''
	echo 'Exit status is 0 if OK, 1 if minor problems, 2 if serious trouble.'
	echo ''
	echo 'Examples:'
	echo '  png_recompress *.png    Runs png_recompress on all *.png files in the current directory.'
	exit $1
}

print_version() {
	echo "png_recompress $png_recompress_version - Script to automate compressing of png images."
	echo ''
	echo 'Copyright (C) 2007-2008 Arvid Norlander'
	echo 'This is free software; see the source for copying conditions.  There is NO'
	echo 'warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.'
	echo ''
	echo 'Written by Arvid Norlander.'
	envbot_quit 0
}

#---------------------------------------------------------------------
## Checks if a space separated list contains a value.
## @param List to check.
## @param Value to check for.
## @return 0 If found.
## @return 1 If not found.
#---------------------------------------------------------------------
list_contains() {
	[[ " ${!1} " = *" $2 "* ]]
}

die() {
	if [[ $2 ]]; then
		echo -e "\e[1mERROR\e[0m: $2"
	fi
	exit $1
}

# Array of paths (images or directories) to process
PATHS=()
# Option: Skip passes
SKIP=""
# Recursive?
RECURSIVE=''
if [[ $# -gt 0 ]]; then
	while [[ $# -gt 0 ]]; do
		case "$1" in
			'--help'|'-help'|'--usage'|'-usage'|'-h')
				print_cmd_help 0
				;;
			'--version'|'-V')
				print_version
				;;
			'--recursive'|'-r')
				RECURSIVE=1
				shift 1
				;;
			'--skip')
				if [[ -z "$2" ]]; then
					die 1 "Missing parameter to --skip. Maybe try --help"
				else
					case $2 in
						1|2|3)
							SKIP+=" $2"
							shift 2
							;;
						*)
							die 1 "Parameter to skip must be 1, 2 or 3 (see --help)"
							;;
					esac
				fi
				;;
			'-'*)
				die 1 "Unknown option, try --help"
				;;
			*)
				PATHS+=( "$1" )
				shift 1
				;;
		esac
	done
else
	print_cmd_help 1
fi

if ! hash optipng > /dev/null 2>&1; then
	echo "Can't find optipng!"
	echo "This script depends on the optipng tool to be in PATH."
	echo "Please install it or, if it is already installed add the"
	echo "directory it is in to PATH and try again."
	echo "Homepage of this tool is: http://optipng.sourceforge.net/"
	die 2
fi

if ! hash advpng > /dev/null 2>&1; then
	echo "Can't find advpng!"
	echo "This script depends on the advpng tool to be in PATH."
	echo "Please install it or, if it is already installed add the"
	echo "directory it is in to PATH and try again."
	echo "Homepage of this tool is: http://advancemame.sourceforge.net/comp-readme.html"
	echo "Hint: For package name in your distro, try looking for \"advancecomp\" if"
	echo "there is no \"advpng\"."
	die 2
fi

echo -e "Please wait, this can take a \e[1mlong\e[0m time."

# Array of image files
IMAGES=()


# ${PATHS[@]} paths to process
# Sets IMAGES
collect_images_recursive() {
	local image directory results
	local images=()
	for directory in "${PATHS[@]}"; do
		if ! [[ -d $directory && -r "$directory" && -w "$directory" ]]; then
			echo "$directory skipped: directory not found, not readable and/or writable, or not a directory."
			continue
		fi
		export IFS=$'\n'
		results=( $(find "$directory" -iname '*.png' -type f -print ) )
		unset IFS
		images+=( "${results[@]}" )
	done
	PATHS=( "${images[@]}" )
}

# This will check that the images in question is a png, otherwise remove it from the list
checkimages() {
	local image
	local failcount=0
	for image in "${PATHS[@]}"; do
		# First check it is a file that exists:
		if ! [[ -f "$image" && -r "$image" && -w "$image" ]]; then
			echo "$image skipped: file not found, not readable and/or writable, or not a normal file"
			(( failcount++ ))
			continue
		elif ! [[ $(file -i "$image" ) == *image/png* ]]; then
			echo "$image skipped: not a valid png (according to file)"
			(( failcount++ ))
			continue
		else
			IMAGES+=( "$image" )
		fi
	done
	if [[ $failcount -ne 0 ]]; then
		echo "Of the images given as parameters $failcount could not be processed (see above for reasons)"
	else
		echo "All images ok"
	fi
	return 0
}

# Run optipng
dooptipng() {
	optipng -i 0 -o 7 "$@" | \
		awk '
			/^\*\* Processing:/ { print "File:   " $3 }
			/^Input file size/ { print "Input:  " $5,$6 }
			/^Output file size/ { print "Output: " $5,$6,$7,$8,$9,$10,$11,"\n" }
			/is already optimized/ { print "Output: No change\n" }
		'
}

# Run advpng
doadvpng() {
	# I hope this matches up with the columns in all cases...
	echo "         In          Out   %  Filename"
	advpng -z -4 "$@"
}

# Check if pass should run, if yes print out the string as description and
# returns true. If no: returns false.
# $1 = pass number
# $2 = string to print
pass_check() {
	if ! list_contains SKIP "$1"; then
		echo -e "\n\e[1mPass $1: $2\e[0m\n"
		return 0
	else
		return 1
	fi
}

if [[ $RECURSIVE ]]; then
	echo "Collecting images from directories"
	collect_images_recursive
fi

pass_check 0 "verifying images (can not be skipped, checks that all images exists and are png)" && {
	checkimages || die 2 "Pass 0 (check images) failed with return code $?"
}

# Hacks around issue with one more newline on optipng
echo

pass_check 1 "optipng" && {
	dooptipng "${IMAGES[@]}" || die 2 "Pass 1 (optipng) failed with return code $?"
}

pass_check 2 "advpng" && {
	doadvpng "${IMAGES[@]}" || die 2 "Pass 2 (advpng) failed with return code $?"
}

# Hacks around issue with one more newline on optipng
echo

pass_check 3 "optipng again (as advpng often makes optipng more effective)" && {
	dooptipng "${IMAGES[@]}" || die 2 "Pass 3 (optipng) failed with return code $?"
}

# Hacks around issue with one more newline on optipng
echo

exit 0
