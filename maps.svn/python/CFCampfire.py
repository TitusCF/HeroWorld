# CFCampfire.py - Talking campfire-related classes
#
# Copyright (C) 2006 Nicolas Weeger
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
#

import Crossfire
import random
import os

key_status = 'campfire_status'
key_line = 'campfire_line'
key_story = 'campfire_story'
status_read = 'read'
status_pause = 'pause'

class CFCampfire:
	'''Main handling class.'''

	def say(self):
		'''Called when the player talks to the firecamp.'''
		w = Crossfire.WhoAmI()

		if ( Crossfire.WhatIsMessage() == 'story' ):
			if w.ReadKey(key_status) == '':
				w.Say('So you want a story, he... All right.')
				if w.ReadKey(key_story) == '':
					w.WriteKey(key_story,self.get_story(), 1)
				w.WriteKey(key_status, status_read, 1)
				w.WriteKey(key_line, '0', 1)
				w.CreateTimer(2,1)
		elif ( Crossfire.WhatIsMessage() == 'pause' ):
			if w.ReadKey(key_status) == status_read:
				w.Say('Sure, let\'s rest some.')
				w.WriteKey(key_status, status_pause, 1)
		elif ( Crossfire.WhatIsMessage() == 'resume' ):
			if w.ReadKey(key_status) == status_pause:
				w.Say('Ok, let\'s continue.')
				w.WriteKey(key_status, status_read, 1)
		elif ( Crossfire.WhatIsMessage() == 'stop' ):
			if w.ReadKey(key_status) != '':
				w.WriteKey(key_status, '', 1)
				w.WriteKey(key_story,'', 1)
				w.Say('Hump, very well, I\'ll keep quiet from now on.')

	def get_story(self):
		'''Returns a random story from the stories directory, internal use.'''
		story_dir = os.path.join( Crossfire.DataDirectory(), 'stories' )
		stories = os.listdir(story_dir)
		return stories[random.randint(0, len(stories) - 1)]

	def timer(self):
		'''Called when the firecamp's timer expire. Sends more of the story.'''
		w = Crossfire.WhoAmI()
		if w.ReadKey(key_status) == '':
			# Been stopped in talking, probably.
			return

		line = int('0' + w.ReadKey(key_line))

		if w.ReadKey(key_status) == status_pause:
			w.CreateTimer(1,1)
			return

		f = open(os.path.join( Crossfire.DataDirectory(), 'stories', w.ReadKey(key_story)), 'r')
		c = f.read().split('\n')
		if line >= len(c):
			w.Say('End of story.')
			w.WriteKey(key_status, '', 1)
			w.WriteKey(key_line, '0', 1)
			w.WriteKey(key_story, '', 1)
		else:
			w.Say(c[line])
			w.CreateTimer(len(c[line]) / 10,1)
			line = line + 1
			w.WriteKey(key_line, str(line), 1)
