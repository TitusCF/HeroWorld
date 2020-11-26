"""
Path utilities for the private transport package

(may be made general and work with other packages)

Here I will make the convertion between:

- Absolute paths: /usr/games/crossfire/share/crossfire/maps/scorn/shops/bank
- Maps paths (relative to the 'maps' directory, in Crossfire format): /scorn/shops/bank

Note: may not work on Windows
"""

import os.path

import Crossfire


MAPS_DIR = os.path.realpath(os.path.join(
    Crossfire.DataDirectory(),
    Crossfire.MapDirectory(),
))


def abs2map(abspath):
    """Convert an absolute path to a path relative to the maps directory, in
    Crossfire format."""

    abspath = os.path.realpath(abspath)
    relpath = os.path.relpath(abspath, MAPS_DIR)

    return '/' + relpath


def map2abs(mappath):
    """Convert a map path (in Crossfire format) to an absolute path"""

    relpath = mappath.lstrip('/')
    abspath = os.path.join(MAPS_DIR, relpath)

    return abspath
