"""
Script that is called when the "Create cabin" scroll is applied.
"""

import os.path

import Crossfire

import private_ship.path as path
from private_ship.common import ActionError
from private_ship.common import InternalError
from private_ship.common import check_if_player_inside_private_transport_they_own
from private_ship.common import generate_feature_id
from private_ship.common import get_player_transport
from private_ship.common import get_private_transport_feature_type
from private_ship.common import set_private_transport_feature_id
from private_ship.common import set_private_transport_feature_type
from private_ship.config import CABINS_DIRECTORY
from private_ship.config import CABIN_KEY_ARCHETYPE
from private_ship.config import CABIN_KEY_GENERATED_COUNT
from private_ship.config import CABIN_KEY_NAME
from private_ship.config import CABIN_KEY_NAME_PL
from private_ship.config import CABIN_LIMIT_DEFAULT
from private_ship.config import EXTERNAL_CABIN_DOOR_ARCHETYPE
from private_ship.config import EXTERNAL_CABIN_DOOR_NAME
from private_ship.config import FEATURE_KEY_SLAYING_PREFIX
from private_ship.config import PLAYER_MESSAGES
from private_ship.config import PRIVATE_TRANSPORT_ATTRS
from private_ship.config import PRIVATE_TRANSPORT_FEATURE_ATTRS
from private_ship.utils import iter_inv
from private_ship.utils import log_error
from private_ship.utils import send_error_to_player
from private_ship.utils import send_info_to_player


# Exceptions
#
# pylint: disable=missing-class-docstring

class CannotCreateMoreCabins(ActionError):
    msg = PLAYER_MESSAGES['cannot_create_more_cabins']


# Logic
#
# pylint: disable=unused-argument

# (player applies "Create cabin" scroll)
# Check if player can use feature scroll (common)
#   If not, inform the user and exit
# If player is in a personal ship he owns and can use the scroll:
#   Create cabin
#   Generate cabin id
#   Create cabin door
#   Generate keys for this cabin

def main():
    """Called code"""

    player = Crossfire.WhoIsActivator()
    scroll = Crossfire.WhoAmI()

    try:
        check_if_player_can_use_scroll(player, scroll)
        use_scroll(player, scroll)
    except InternalError as err:
        log_error(err.errmsg)
        send_error_to_player(err.msg, player)
        return
    except ActionError as err:
        send_error_to_player(err.msg, player)
        return
    except Exception as err:
        send_error_to_player(InternalError.msg, player)
        raise


def check_if_player_can_use_scroll(player, scroll):
    """Verify if player can use scroll

    If the player cannot use the scroll, raise an ActionError, which
    will contain the appropriate message to send to the player.

    Otherwise (if the player can use the scroll and it would take
    effect), this function is NOP.
    """

    # player must be inside a private transport that they own
    check_if_player_inside_private_transport_they_own(player)

    # number of cabins after applying the scroll must be <= cabin limit
    transport = get_player_transport(player)
    cabin_limit = get_cabin_limit(transport)
    current_cabin_count = get_current_cabin_count(transport)
    if current_cabin_count + 1 > cabin_limit:
        raise CannotCreateMoreCabins

    # everything okay


def get_cabin_limit(transport):
    """Get the maximum amount of cabins that a transport can
    accomodate."""

    value = transport.ReadKey(PRIVATE_TRANSPORT_ATTRS['limit_cabins'])

    # check for default value
    if not value:
        return CABIN_LIMIT_DEFAULT

    # check for invalid value
    # .isdigit() check if all characters are digits, that way, `value` will
    # only be valid if it is a str representing an integer
    if not value.isdigit():
        log_error(
            "Private Transport: "
            "Cannot read cabins limit '%s'. "
            "Using default value instead..." % (value)
        )
        return CABIN_LIMIT_DEFAULT

    # convert to int and return
    value = int(value)
    return value


def get_current_cabin_count(transport):
    """Discover how many cabins are in a transport right now."""

    return sum(is_cabin_door(o) for o in iter_inv(transport))


def is_cabin_door(obj):
    """Return True if obj is a cabin door, False otherwise."""
    return get_private_transport_feature_type(obj) == 'cabin'


def use_scroll(player, scroll):
    """Use the "Create cabin" scroll

    After this operation, we will have:

    - A cabin map built for the player
    - A door inside the transport that leads to the cabin
    - Keys that allow a non-owner of the transport to go to the cabin

    The keys only work with the specific cabin door.

    Oh, and the scroll will be removed :)
    """

    # Generate cabin id
    id_ = generate_feature_id()

    # Create cabin map
    map_location = create_cabin_map(player, scroll, id_)

    # Create cabin door and add it to the transport's inventory
    create_cabin_door(player, scroll, map_location, id_)

    # Remove the player and put him back on the transport so that the
    # added cabin becomes visible
    transport = get_player_transport(player)
    transport.Apply(player, 0)
    transport.Apply(player, 0)

    # Create keys and add them to player inventory
    create_cabin_keys(player, scroll, id_)

    # Vanish with the scroll
    # Note: Remove() will remove all stacked scrolls at once
    scroll.Quantity -= 1

    # Inform the player
    inform_player_of_new_cabin_added(player, scroll, id_)


def create_cabin_map(player, scroll, id_):
    """Generate a new cabin map, based on a template, and return the
    path to the generated map.

    Returns:
        (str) An absolute path to the generated map.
    """

    # pylint: disable=invalid-name

    # Decide location to save map
    map_location = get_map_location_for_cabin(player, scroll, id_)

    # Decide template to use
    template_file = decide_template_to_use(player, scroll)

    # Read contents of template
    with open(template_file, mode='r') as fp:
        text = fp.read()

    # Create new cabin file
    with open(map_location, mode='w') as fp:
        fp.write(text)

    # Return the location of the cabin file
    return map_location


def get_map_location_for_cabin(player, scroll, id_):
    """Get path that a certain cabin will be saved to."""

    return os.path.join(
        CABINS_DIRECTORY,
        id_,
    )


def decide_template_to_use(player, scroll):
    """Decide which template to use when creating a cabin map

    Returns an *absolute* path to the template file.
    """

    transport = get_player_transport(player)
    template = transport.ReadKey(PRIVATE_TRANSPORT_ATTRS['cabin_template'])
    if not template:
        raise InternalError(
            "Failed to create cabin. Private transport without \"%s\" property."
            "" % PRIVATE_TRANSPORT_ATTRS['cabin_template']
        )
    template_abs = path.map2abs(template)

    return template_abs


def create_cabin_door(player, scroll, map_location, id_):
    """Create a door that leads to the cabin and add it to the transport

    Args:
        player
        scroll
        map_location: Absolute path corresponding to the cabin

    Returns:
        A reference to the created door.
    """

    transport = get_player_transport(player)

    # Create door inside transport's inventory
    door = transport.CreateObject(EXTERNAL_CABIN_DOOR_ARCHETYPE)
    door.WriteKey(PRIVATE_TRANSPORT_FEATURE_ATTRS['flag'], '1', 1)
    set_private_transport_feature_type(door, 'cabin')
    set_private_transport_feature_id(door, id_)

    # Set door name
    door.Name = EXTERNAL_CABIN_DOOR_NAME.format(id_=id_)
    door.NamePl = EXTERNAL_CABIN_DOOR_NAME.format(id_=id_)

    # Link door to cabin
    door.Slaying = path.abs2map(map_location)
    door.HP = int(transport.ReadKey(PRIVATE_TRANSPORT_ATTRS['cabin_entrance_x']))
    door.SP = int(transport.ReadKey(PRIVATE_TRANSPORT_ATTRS['cabin_entrance_y']))

    return door


def create_cabin_keys(player, scroll, id_):
    """Create cabin keys and insert them into the player's inventory"""

    keys = player.CreateObject(CABIN_KEY_ARCHETYPE)
    keys.Slaying = FEATURE_KEY_SLAYING_PREFIX + id_
    keys.Quantity = CABIN_KEY_GENERATED_COUNT
    keys.Name = CABIN_KEY_NAME.format(id_=id_)
    keys.NamePl = CABIN_KEY_NAME_PL.format(id_=id_)


def inform_player_of_new_cabin_added(player, scroll, id_):
    """Inform player that a new cabin has just been created."""

    send_info_to_player(
        PLAYER_MESSAGES['cabin_created'],
        player,
    )


# prevent scroll apply
Crossfire.SetReturnValue(1)

# run script
main()
