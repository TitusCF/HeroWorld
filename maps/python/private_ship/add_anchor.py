"""
Script called when an "Add anchor" scroll is used
"""

import Crossfire
from private_ship.common import ActionError
from private_ship.common import InternalError
from private_ship.common import check_if_player_inside_private_transport_they_own
from private_ship.common import generate_feature_id
from private_ship.common import get_player_transport
from private_ship.common import get_private_transport_feature_type
from private_ship.common import set_private_transport_feature_flag
from private_ship.common import set_private_transport_feature_id
from private_ship.common import set_private_transport_feature_type
from private_ship.config import ANCHOR_ARCHETYPE
from private_ship.config import ANCHOR_KEY_ARCHETYPE
from private_ship.config import ANCHOR_KEY_GENERATED_COUNT
from private_ship.config import ANCHOR_KEY_NAME
from private_ship.config import ANCHOR_KEY_NAME_PL
from private_ship.config import ANCHOR_NAME
from private_ship.config import FEATURE_KEY_SLAYING_PREFIX
from private_ship.config import PLAYER_MESSAGES
from private_ship.utils import send_error_to_player
from private_ship.utils import send_info_to_player
from utils import iter_inv
from utils import log_error


# Exceptions
#
# pylint: disable=missing-class-docstring

class CanOnlyHaveOneAnchor(ActionError):
    msg = PLAYER_MESSAGES['can_only_have_one_anchor']


# Logic

# (player applies "Add anchor" scroll)
# if player can create an anchor
  # create anchor
  # add it to private transport inventory
# otherwise
  # inform the player and exit

def main():
    """Main script"""

    player = Crossfire.WhoIsActivator()
    scroll = Crossfire.WhoAmI()

    try:
        check_if_player_can_use_scroll(player, scroll)
        use_scroll(player, scroll)
    # TODO: this error handling could be common
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

    If player can't use scroll, raise an ActionError.
    """

    # player must be inside a private transport they own
    check_if_player_inside_private_transport_they_own(player)

    # there can only be 1 anchor per ship
    transport = get_player_transport(player)
    num_anchors = get_number_of_anchors(transport)
    if num_anchors >= 1:
        raise CanOnlyHaveOneAnchor

    # everything fine


def use_scroll(player, scroll):
    """Use the "Add anchor" scroll"""

    # Generate anchor id
    id_ = generate_feature_id()

    # Create andhor and add it to the transport's inventory
    create_anchor(player, scroll, id_)

    # Create keys and add them to player's inventory
    create_anchor_keys(player, scroll, id_)

    # Inform the player
    send_info_to_player(
        PLAYER_MESSAGES['anchor_added'],
        player,
    )

    # Remove the scroll
    scroll.Quantity -= 1

    # Get player out and into transport mode to display the new feature
    transport = get_player_transport(player)
    transport.Apply(player, 0)
    transport.Apply(player, 0)


def get_number_of_anchors(transport):
    """Get the number of anchors in a private transport"""
    return sum(is_anchor(obj) for obj in iter_inv(transport))


def is_anchor(obj):
    """Check if object_ is an anchor"""
    return get_private_transport_feature_type(obj) == 'anchor'


def create_anchor(player, scroll, id_):
    """Create an anchor with a certain id_

    Returns:
        A reference to the newly created anchor object.
    """

    transport = get_player_transport(player)

    # Create anchor
    anchor = transport.CreateObject(ANCHOR_ARCHETYPE)
    set_private_transport_feature_flag(anchor)
    set_private_transport_feature_type(anchor, 'anchor')
    set_private_transport_feature_id(anchor, id_)

    # Set anchor name (so we can match it with key)
    anchor.Name = ANCHOR_NAME.format(id_=id_)
    anchor.NamePl = ANCHOR_NAME.format(id_=id_)

    return anchor


def create_anchor_keys(player, scroll, id_):
    """Create keys for an anchor with ID id_"""

    keys = player.CreateObject(ANCHOR_KEY_ARCHETYPE)
    keys.Slaying = FEATURE_KEY_SLAYING_PREFIX + id_
    keys.Quantity = ANCHOR_KEY_GENERATED_COUNT
    keys.Name = ANCHOR_KEY_NAME.format(id_=id_)
    keys.NamePl = ANCHOR_KEY_NAME_PL.format(id_=id_)


# prevent scroll apply
Crossfire.SetReturnValue(1)

main()
