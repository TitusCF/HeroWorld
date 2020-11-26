"""
Script used to teleport the player to the cabin when he uses an external
cabin door.

See also: create_cabin.py
"""

import Crossfire

import tracking_system
from private_ship.common import ActionError
from private_ship.common import InternalError
from private_ship.common import check_if_player_can_use_feature
from private_ship.common import get_player_transport
from private_ship.config import INTERNAL_CABIN_DOOR_ARCHETYPE
from private_ship.config import INTERNAL_CABIN_DOOR_ATTR_TRACKING_ID
from private_ship.config import PRIVATE_TRANSPORT_ATTRS
from private_ship.utils import log_error
from private_ship.utils import send_error_to_player
from utils import iter_loc


# (player applies cabin door)
# Check if player can use feature (common)
#   If not, inform the user and exit
# If player is the owner or if he has a key
#   Put player out of transport mode
#   Teleport him to the cabin (map and coordinates)
#     map and coordinates are saved in the door
#   Keep track of transport or transport location


def main():
    """Main"""

    player = Crossfire.WhoIsActivator()
    door = Crossfire.WhoAmI()

    try:
        check_if_player_can_use_door(player, door)
        use_door(player, door)
    except InternalError as err:
        log_error(err.errmsg)
        send_error_to_player(err.msg, player)
    except ActionError as err:
        send_error_to_player(err.msg, player)
    except Exception:
        send_error_to_player(InternalError.msg, player)
        raise


def check_if_player_can_use_door(player, door):
    """Check if the player can use door

    If not possible, raise ActionError with the appropriate message for
    the player.
    """
    check_if_player_can_use_feature(player, door)


def use_door(player, door):
    """Use the door

    This will teleport the player inside the cabin and start tracking
    the ship, so the player can return to it afterwards.
    """

    transport = get_player_transport(player)

    # put player out of transport mode
    #   (otherwise the client breaks down)
    transport.Apply(player, 0)

    # teleport player to the cabin

    # get cabin map and entrance
    cabin_map = door.Slaying
    cabin_x = door.HP
    cabin_y = door.SP
    # prepare map
    map_ = Crossfire.ReadyMap(cabin_map)
    # teleport player
    player.Teleport(map_, cabin_x, cabin_y)

    # start tracking transport and save tracking ID to internal cabin door
    transport_tracking_id = tracking_system.add_tracker(transport)
    internal_cabin_door = find_internal_cabin_door(transport, map_)
    internal_cabin_door.WriteKey(
        INTERNAL_CABIN_DOOR_ATTR_TRACKING_ID,
        transport_tracking_id,
        1,
    )


def find_internal_cabin_door(transport, cabin_map):
    """Find internal cabin door located at cabin_map

    Returns:
        The cabin door object, if it is found, None, otherwise.
    """

    # Get coordinates for the door
    door_x = int(transport.ReadKey(PRIVATE_TRANSPORT_ATTRS['cabin_exit_x']))
    door_y = int(transport.ReadKey(PRIVATE_TRANSPORT_ATTRS['cabin_exit_y']))
    # Find cabin door
    for obj in iter_loc(cabin_map, door_x, door_y):
        if obj.Archetype.Name == INTERNAL_CABIN_DOOR_ARCHETYPE:
            return obj
    # Return None if not found
    return None


Crossfire.SetReturnValue(1)
main()
