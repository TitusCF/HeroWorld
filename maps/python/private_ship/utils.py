"""
Utilities for the private transports programs
"""

import Crossfire

from private_ship.config import ERROR_MESSAGE_FLAGS
from private_ship.config import INFORMATION_MESSAGE_FLAGS


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


def send_error_to_player(msg, player):
    """Send error message to a player"""
    player.Message(msg, ERROR_MESSAGE_FLAGS)


def send_info_to_player(msg, player):
    """Send information message to a player"""
    player.Message(msg, INFORMATION_MESSAGE_FLAGS)
