"""
Ship rename scroll

I run when a "Rename ship" scroll is applied.

Private transports comes in 2 states:

- Ownerless
- Owned by someone

Private transports are transports denoted with the attribute:

    private_transport 1

Note: I was using the term "private ship", but now I believe "private
transport" is a better option and more general with what the game has to
offer. There may be a mix, but always tending towards using transport.

Original psedocode:
  (player applies the scroll)
  (this script is called)
  VALIDATE ACTION
  if scroll is not renamed
    raise ScrollNotRenamed
  if player not aboard anything
    raise PlayerNotInsideTransport
  if player aboard something which is not a private ship
    raise PlayerInsideSomethingElse
  if player aboard a private ship with a different owner
    raise NotOwnerOfTheShip
  (player is owner of the ship)
  EXECUTE ACTION
  if player aboard his own private ship
    rename the ship
    inform the player
  if player aboard an ownerless private ship
    rename the ship
    set the owner of the ship
    inform the player
"""


import Crossfire

from private_ship.common import ActionError, InternalError
from private_ship.common import check_if_player_inside_private_transport_they_own_or_can_own
from private_ship.common import get_player_transport
from private_ship.common import is_transport_owned_by_player
from private_ship.config import NEW_TRANSPORT_NAME
from private_ship.config import PLAYER_MESSAGES
from private_ship.config import PRIVATE_TRANSPORT_ATTRS as ATTRS
from private_ship.utils import send_error_to_player
from private_ship.utils import send_info_to_player
from utils import log_error


# Exceptions

class ScrollNotRenamed(ActionError):
    msg = PLAYER_MESSAGES['scroll_not_renamed']


# Logic

def main():
    """Execute the program when a Ship Rename Scroll is used
    """

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
    """Check if a player can use the Ship Rename Scroll

    NOOP if they can, throw an ActionError if they can't.
    """

    scroll_name = get_scroll_custom_name(scroll)
    if not scroll_name:
        raise ScrollNotRenamed

    check_if_player_inside_private_transport_they_own_or_can_own(player)


def use_scroll(player, scroll):
    """Make a player use a Ship Rename Scroll

    Usage has already been validated, so player is on a private ship,
    either one which is ownerless, or one which the player owns.
    """
    transport = get_player_transport(player)
    if is_transport_owned_by_player(transport, player):
        use_scroll_already_owned(player, scroll)
    else:
        use_scroll_new(player, scroll)


def use_scroll_already_owned(player, scroll):
    """Functionality for using a scroll when player already owns the transport

    - rename the ship
    - inform the player
    - remove scroll
    - put player out and into transport mode so the name of the ship updates
    """

    transport = get_player_transport(player)

    scroll_name = get_scroll_custom_name(scroll)
    transport_name = get_transport_name(scroll_name, player.Name)

    rename_transport(transport, transport_name)

    send_info_to_player(
        PLAYER_MESSAGES['transport_renamed'].format(transport_name=scroll_name),
        player)

    # Move player out of transport and then into transport
    # This way, the new name of the transport will appear
    transport.Apply(player, 0)
    transport.Apply(player, 0)

    scroll.Quantity -= 1


def use_scroll_new(player, scroll):
    """Functionality for using a scroll when the transport is ownerless

    - rename the transport
    - set player as the owner
    - inform the player
    - remove scroll
    - put player out and into transport mode so the name of the ship updates
    """

    transport = get_player_transport(player)
    scroll_name = get_scroll_custom_name(scroll)
    transport_name = get_transport_name(scroll_name, player.Name)

    rename_transport(transport, transport_name)
    set_transport_owner(transport, player)

    send_info_to_player(
        PLAYER_MESSAGES['transport_renamed'].format(transport_name=scroll_name),
        player)
    send_info_to_player(
        PLAYER_MESSAGES['transport_is_mine'].format(transport_name=scroll_name),
        player)

    scroll.Quantity -= 1

    # Move player out of transport and then into transport
    # This way, the new name of the transport will appear
    transport.Apply(player, 0)
    transport.Apply(player, 0)


def get_scroll_custom_name(scroll):
    """Get the scroll custom_name

    custom_name is set with the rename command
    """
    return scroll.CustomName


def get_transport_name(scroll_name, player_name):
    """Get the name of a transport"""
    return NEW_TRANSPORT_NAME.format(
        scroll_name=scroll_name,
        player_name=player_name,
    )


def rename_transport(transport, name):
    """Rename the transport
    """
    transport.Name = name
    transport.NamePl = name


def set_transport_owner(transport, player):
    """Set player as the owner of transport"""
    transport.WriteKey(
        ATTRS['owner_name'],
        player.Name,
        1,
    )


# prevent scroll apply
Crossfire.SetReturnValue(1)

# run script
main()
