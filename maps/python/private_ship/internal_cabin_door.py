"""
Script used to teleport the player back from a cabin to the transport.

Activated when the players applies the door found in the cabin (internal door).
"""

import Crossfire

import tracking_system
from private_ship.config import INTERNAL_CABIN_DOOR_ATTR_TRACKING_ID
from private_ship.config import PLAYER_MESSAGES
from private_ship.utils import send_info_to_player
from utils import log_error


# (player attempts to leave cabin)
# Get tracking ID from the internal door
# Check if we managed to keep track of the ship location
#   If not, teleport player to their home, apologize and exit
# If we kept track of the ship
#   Teleport player to ship location
#   Put player in transport mode

# ~~Stop tracking ship~~
#   Do not stop tracking ship. The tracking system currently maintains
#   only a number of trackers (it doesn't know WHO is tracking). So, say
#   a players starts tracking a shipp, a DM that teleported to him could
#   make this ship stop being tracked, which could cause a problem.
#
#   A bad side-effect is that the number of trackers will only increase,
#   so a ship will never stop being tracked. This can be bad for good
#   amounts of players.


def main():
    """Main script"""
    player = Crossfire.WhoIsActivator()
    door = Crossfire.WhoAmI()

    id_ = get_tracking_id(door)
    transport = tracking_system.get_object(id_)
    if transport is None:
        log_error('Lost private transport with tracking id %s' % id_)
        log_error('Player affected: %s' % player.Name)
        send_info_to_player(PLAYER_MESSAGES['lost_track_of_transport'], player)
        teleport_player_to_bed(player)
    else:
        teleport_player_to_transport(player, transport)
        transport.Apply(player, 0)


def get_tracking_id(door):
    """ Return the transport tracking ID found on the door, or ''"""
    return door.ReadKey(INTERNAL_CABIN_DOOR_ATTR_TRACKING_ID)


def teleport_player_to_bed(player):
    """Teleport player to their bed"""

    mapname = player.BedMap
    bedx = player.BedX
    bedy = player.BedY

    flags = 2 if Crossfire.PlayerDirectory() in mapname else 0

    map_ = Crossfire.ReadyMap(mapname, flags)
    player.Teleport(map_, bedx, bedy)


def teleport_player_to_transport(player, transport):
    """Teleport a player to a transport

    Assumes the transport is already loaded with its map.
    """
    player.Teleport(transport.Map, transport.X, transport.Y)


main()
