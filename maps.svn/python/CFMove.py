# CFMove.py - various move-related functions.
#
# Copyright (C) 2007 Nicolas Weeger
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


'''Those represents the x/y offsets to move to a specific direction.'''
dir_x = [  0, 0, 1, 1, 1, 0, -1, -1, -1 ]
dir_y = [ 0, -1, -1, 0, 1, 1, 1, 0, -1 ]

def coordinates_to_dir(x, y):
	'''This returns, for the specified offsets, the direction to move to get there.'''
	q = 0
	if(y == 0):
		q = -300 * x;
	else:
		q = int(x * 100 / y);
	if(y>0):
		if(q < -242):
			return 3
		if (q < -41):
			return 2
		if (q < 41):
			return 1
		if (q < 242):
			return 8 ;
		return 7

	if (q < -242):
		return 7
	if (q < -41):
		return 6
	if (q < 41):
		return 5
	if (q < 242):
		return 4
	return 3

def get_object_to(obj, x, y):
	'''This tries to move obj to the (x, y) location.
	Return value is:
	 * 0: object is on the spot
	 * 1: object moved towards the goal
	 * 2: object's movement was blocked.
	 '''
	if obj.X == x and obj.Y == y:
		return 0
	# Move returns 0 for couldn't move, 1 for moved.
	return 2 - obj.Move(coordinates_to_dir(obj.X - x, obj.Y - y))
