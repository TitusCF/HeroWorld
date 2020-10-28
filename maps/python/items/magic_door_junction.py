'''
This script runs when the magic door is applied

- generate new map
- link door to map
'''

import errno
import os
import os.path
import sys
from itertools import count

import Crossfire


# Classes

class Path(object):

    '''I am a path

    You can view the absolute and relative versions of me, the
    relative version is rooted at the "maps" directory.
    '''

    def __init__(self, rel_path=None, abs_path=None):

        # will be set by setters
        self._apath = None

        abs_path_present = abs_path is not None
        rel_path_present = rel_path is not None
        if abs_path_present and not rel_path_present:
            self.absolute = abs_path
        elif not abs_path_present and rel_path_present:
            self.relative = rel_path
        elif not abs_path_present and not rel_path_present:
            raise ValueError(
                "At least one of abs_path or rel_path must be not "
                "null.")
        else:
            raise ValueError(
                "Please use only the abs_path or rel_path, not "
                "both at the same time.")

    def __repr__(self):
        return 'Path({!r})'.format(self.relative)

    @property
    def absolute(self):
        return self._apath

    @absolute.setter
    def absolute(self, val):
        self._apath = val

    @property
    def relative(self):
        return self._abs_to_rel(self.absolute)

    @relative.setter
    def relative(self, val):
        self.absolute = self._rel_to_abs(val)

    @classmethod
    def get_maps_dir(cls):
        '''Retrieve the location of crossfire "maps" directory'''
        return os.path.dirname(sys.path[0])

    @classmethod
    def _abs_to_rel(cls, apath):
        '''Convert absolute path to relative'''
        return '/' + os.path.relpath(
            os.path.realpath(apath),
            os.path.realpath(cls.get_maps_dir())
        )

    @classmethod
    def _rel_to_abs(cls, rpath):
        '''Convert relative path to absolute'''
        return os.path.join(
            os.path.realpath(cls.get_maps_dir()),
            rpath[1:] if rpath.startswith('/') else rpath,
        )


class Template(object):

    '''I'm a template

    I can be used to instantiate a map
    '''

    def __init__(self, entrance_x, entrance_y, path, text=None):
        self.entrance_x = entrance_x
        self.entrance_y = entrance_y
        self.path = path
        self._text = text

    @property
    def text(self):
        if self._text is None:
            with open(self.path.absolute) as fp:
                self._text = fp.read()
        return self._text

    def instantiate(self, player, door):
        '''Create a map using myself

        This will return a Map object with the map file already saved.
        '''

        # get map path
        path = Map.generate_new_path_for(player, door)

        # get map text
        text = self.text.format(
            EXIT_MAP=door.Map.Path,
            EXIT_X=door.X,
            EXIT_Y=door.Y,
        )

        # make sure directory exists
        try:
            os.makedirs(os.path.dirname(path.absolute))
        except OSError as error:
            if error.errno != errno.EEXIST:
                raise

        # save text to file
        with open(path.absolute, mode='w') as fp:
            fp.write(text)

        # return new map object
        map_ = Map(self.entrance_x, self.entrance_y, path, text)
        return map_


class Map(object):

    '''I'm a map'''

    def __init__(self, entrance_x, entrance_y, path, text=None):
        self.entrance_x = entrance_x
        self.entrance_y = entrance_y
        self.path = path
        self._text = text

    @property
    def text(self):
        if self._text is None:
            with open(self.path.absolute) as fp:
                self._text = fp.read()
        return self._text

    @staticmethod
    def generate_new_path_for(player, door):
        '''Generate a new map path for a player/door combination'''

        # the generated map is a child of the map the door was in

        door_map_path = door.Map.Path

        # try 01 as a suffix
        # if fail,
        #   try 02 as a suffix
        # if fail,
        #  try 03 as a suffix
        # and so on...
        for number in count(1):
            path = Path('{}~~{}'.format(door_map_path, number))
            if not os.path.exists(path.absolute):
                # a running condition could cause 2 names to be the same
                # this would trickle down into 2 doors being generated pointed at the same map
                break

        return path


# Constants

# set your templates here

TEMPLATES = {
    'magicdoor': Template(8, 15, Path('/templates/buildable/additions/basic_junction')),
}

# maximum depth that magic doors can go to
# MAX_DEPTH = 1 allows only the creation of superficial doors
# MAX_DEPTH = 0 disallows the creation of magic doors
MAX_DEPTH = 8


# Script

def main():

    door = Crossfire.WhoAmI()
    player = Crossfire.WhoIsActivator()

    # Door properties
    # door.Slaying : relative path to map file
    #   > if not present, it means the door has not been activated yet
    # door.HP : X position to teleport the player to
    # door.SP : Y position to teleport the player to

    # calculate magic doors' depth
    # if too deep, inform player, remove door and exit early
    # Note: this will fail if the user name contains ~~.
    #        > the effect is that the player with ~~ in the name may be unable
    #        > to create magic doors
    depth = os.path.basename(door.Map.Path).count('~~')
    if depth + 1 > MAX_DEPTH:
        flags = Crossfire.MessageFlag.NDI_UNIQUE + Crossfire.MessageFlag.NDI_RED
        player.Message("Sorry, you cannot go deeper.", flags)
        door.Remove()
        return

    # if door was never used, create map and link door to it
    if not door.Slaying:
        map_ = generate_map(player, door)
        door.Slaying = map_.path.relative
        door.HP = map_.entrance_x
        door.SP = map_.entrance_y

    # teleport player to map
    m = Crossfire.ReadyMap(door.Slaying)
    player.Teleport(m, door.HP, door.SP)


def generate_map(player, door):
    '''Generate a new map for the magic door.'''
    template = choose_template(player, door)
    map_ = template.instantiate(player, door)
    return map_


def choose_template(player, door):
    '''Choose template that will be used for combination of player and door'''
    return TEMPLATES['magicdoor']


# RUN

main()
