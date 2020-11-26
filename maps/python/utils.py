"""
I have general utilities.
"""

import Crossfire


def iter_loc(map_, coordinate_x, coordinate_y):
    """Iterate over all object at location x/y on map_"""
    obj = map_.ObjectAt(coordinate_x, coordinate_y)
    while obj is not None:
        yield obj
        obj = obj.Above


def iter_inv(obj):
    """Iterate over all the objects in the inventory of an object"""
    cur = obj.Inventory
    while cur:
        yield cur
        cur = cur.Below


def log_error(msg):
    """Log error message"""
    if not isinstance(msg, str):
        msg = repr(msg)
    Crossfire.Log(Crossfire.LogError, msg)


def log_info(msg):
    """Log info message"""
    if not isinstance(msg, str):
        msg = repr(msg)
    Crossfire.Log(Crossfire.LogInfo, msg)


def log_debug(msg):
    """Log debug message (visible from debug mode)"""
    if not isinstance(msg, str):
        msg = repr(msg)
    Crossfire.Log(Crossfire.LogDebug, msg)
