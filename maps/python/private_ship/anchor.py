"""
Script run when a private ship anchor is applied.
"""

import Crossfire

from private_ship.common import ActionError
from private_ship.common import InternalError
from private_ship.common import check_if_player_can_use_feature
from private_ship.common import get_player_transport
from private_ship.config import PLAYER_MESSAGES
from private_ship.config import PRIVATE_TRANSPORT_ATTRS
from private_ship.utils import send_error_to_player
from private_ship.utils import send_info_to_player
from utils import log_error



# (player applies anchor)
# check if player can use feature (common)
#   if not, inform the user and exit
# If player can use feature
#   toggle anchored status of private ship
#     > private_transport_anchored
#   set speed accordingly
#     > private_transport_base_speed
#     > speed

def main():
    """Main script"""

    player = Crossfire.WhoIsActivator()
    anchor = Crossfire.WhoAmI()

    try:
        check_if_player_can_use_anchor(player, anchor)
        use_anchor(player, anchor)
    except InternalError as err:
        log_error(err.errmsg)
        send_error_to_player(err.msg, player)
        return
    except ActionError as err:
        send_error_to_player(err.msg, player)
        return
    except Exception:
        send_error_to_player(InternalError.msg, player)
        raise


def check_if_player_can_use_anchor(player, anchor):
    """Verify if player can use anchor

    If player can't use, raise an ActionError, otherwise, NOOP.
    """
    check_if_player_can_use_feature(player, anchor)


def use_anchor(player, anchor):
    """Use an anchor"""

    transport = get_player_transport(player)
    toggle_anchored(player, transport, anchor)


def toggle_anchored(player, transport, anchor):
    """Toggle anchored status of a transport"""

    if is_transport_anchored(transport):
        deactivate_anchor(player, transport, anchor)
    else:
        activate_anchor(player, transport, anchor)


def activate_anchor(player, transport, anchor):
    """Activate the anchor of a ship"""
    # set anchored state
    transport.WriteKey(PRIVATE_TRANSPORT_ATTRS['anchored'], '1', 1)
    # set speed to 0
    transport.Speed = 0.
    # I don't know the reason, but, when Speed is set to 0, the
    # transport can still move one tile. Setting SpeedLeft to a negative
    # value stops this from happening.
    transport.SpeedLeft = -0.01
    # notify the user that an anchor is being used
    send_info_to_player(
        PLAYER_MESSAGES['transport_anchor_activated'],
        player,
    )


def deactivate_anchor(player, transport, anchor):
    """Deactivate anchor of a ship"""
    # set anchored state
    transport.WriteKey(PRIVATE_TRANSPORT_ATTRS['anchored'], '0', 1)
    # set speed as base speed
    try:
        base_speed = float(transport.ReadKey(PRIVATE_TRANSPORT_ATTRS['base_speed']))
    except ValueError:
        send_error_to_player("There was a problem while deactivating the anchor of "
                             "your ship. It may become slower than usual, "
                             "a DM may help you with it.", player)
        log_error("%s is using an anchor on a private transport without base speed. "
                  "Setting transport speed to 0.5." % player.Name)
        base_speed = 0.5
    transport.Speed = base_speed
    # notify the user that the anchor is lifted
    send_info_to_player(
        PLAYER_MESSAGES['transport_anchor_deactivated'],
        player,
    )


def is_transport_anchored(transport):
    """Return True if transport is anchored, False otherwise"""
    return transport.ReadKey(PRIVATE_TRANSPORT_ATTRS['anchored']) == '1'


# prevent anchor usage
Crossfire.SetReturnValue(1)

main()
