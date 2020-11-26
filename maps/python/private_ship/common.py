"""
Common functions for the private transport
"""

import private_ship.idgen as idgen
from private_ship.config import FEATURE_KEY_SLAYING_PREFIX
from private_ship.config import PLAYER_MESSAGES
from private_ship.config import PRIVATE_TRANSPORT_ATTRS
from private_ship.config import PRIVATE_TRANSPORT_FEATURE_ATTRS


# Exceptions
#
# pylint: disable=missing-class-docstring

class ActionError(Exception):
    """Error that occurs when the player cannot execute an action

    Raise me when the player cannot execute the action he wants!

    Raising me should stop the script from running and send an error
    message to the player.

    Attributes:
        msg: A message that will be displayed to the player.
    """
    msg = PLAYER_MESSAGES['action_error_basic']


class InternalError(ActionError):
    """User cannot execute action but because something happened on the
    server side!
    """
    msg = PLAYER_MESSAGES['internal_error']

    def __init__(self, error_message):
        """Initialize an internal error

        Args:
            error_message: A message to log to the server when this
            error occurs.
        """
        super(InternalError, self).__init__()
        self.errmsg = error_message


class PlayerNotInsideTransport(ActionError):
    msg = PLAYER_MESSAGES['player_not_inside_transport']


class PlayerInsidePublicTransport(ActionError):
    msg = PLAYER_MESSAGES['player_inside_public_transport']


class NotOwnerOfTheTransport(ActionError):
    msg = PLAYER_MESSAGES['not_owner_of_transport']


class TransportHasNoOwner(NotOwnerOfTheTransport):
    msg = PLAYER_MESSAGES['transport_has_no_owner']


class TransportOwnedBySomeoneElse(NotOwnerOfTheTransport):
    msg = PLAYER_MESSAGES['transport_owned_by_someone_else']


class PlayerCannotUseFeature(ActionError):
    msg = PLAYER_MESSAGES['player_cannot_use_feature']


# Utilities

def check_if_player_inside_transport(player):
    """Verify if player is inside a transport."""

    transport = get_player_transport(player)
    if not transport:
        raise PlayerNotInsideTransport


def check_if_player_inside_private_transport(player):
    """Verify if player is inside a private transport."""

    check_if_player_inside_transport(player)

    transport = get_player_transport(player)
    if not is_private_transport(transport):
        raise PlayerInsidePublicTransport


def check_if_player_inside_private_transport_they_own(player):
    """Verify if player is inside a private transport they own."""

    check_if_player_inside_private_transport(player)

    transport = get_player_transport(player)
    if not is_transport_owned_by_player(transport, player):
        raise NotOwnerOfTheTransport


def check_if_player_inside_private_transport_they_own_or_can_own(player):
    """Verify if player is inside a private transport they own or can
    own (ownerless).

    NOP if the transport is ownerless or if the player already owns the
    transport.
    """

    check_if_player_inside_private_transport(player)

    transport = get_player_transport(player)
    if (is_transport_owned(transport) and
            not is_transport_owned_by_player(transport, player)):
        raise TransportOwnedBySomeoneElse


def check_if_player_can_use_feature(player, feature):
    """Verify if player can use a private transport feature

    They will be allowed in two conditions:

    - They are the owner of the transport
    - They have a key to use the feature
    """

    transport = get_player_transport(player)

    if (not is_transport_owned_by_player(transport, player) and
            not does_player_have_feature_key(player, feature)):
        raise PlayerCannotUseFeature


def does_player_have_feature_key(player, feature):
    """Return whether the player has a key to the feature.

    Objects:

    - feature:
          id
    - key:
          slaying (linked to feature id)
    """

    id_ = feature.ReadKey(PRIVATE_TRANSPORT_FEATURE_ATTRS['id'])
    assert id_ != ''
    slaying = FEATURE_KEY_SLAYING_PREFIX + id_
    return player.CheckInventory(slaying) is not None


def get_player_transport(player):
    """Return the transport the player is in, or None, if player is not
    insed a transport."""
    # This actually returns anything the player is inside
    return player.Env


def is_private_transport(transport):
    """Return whether transport is a private transport or not
    """
    return transport.ReadKey(PRIVATE_TRANSPORT_ATTRS['flag']) == '1'


def is_transport_owned(transport):
    """Return whether a private transport is owned or not
    """
    return get_transport_owner_name(transport) is not None


def is_transport_owned_by_player(transport, player):
    """Check if a transport is owned by a player"""
    return get_transport_owner_name(transport) == player.Name


def get_transport_owner_name(transport):
    """Return the name of the transport owner or None, if none"""
    return transport.ReadKey(PRIVATE_TRANSPORT_ATTRS['owner_name']) or None


def get_private_transport_feature_type(feature):
    """Get the type of feature an object is.

    If the object is not a feature, return ''.
    """
    if not is_private_transport_feature(feature):
        return ''
    return feature.ReadKey(PRIVATE_TRANSPORT_FEATURE_ATTRS['type'])


def set_private_transport_feature_flag(feature, value='1'):
    """Set the private transport feature flag for this object

    '1' indicates a private transport feature
    '0' or '' indicates not a feature

    Default value indicates to set as a feature.
    """
    feature.WriteKey(PRIVATE_TRANSPORT_FEATURE_ATTRS['flag'], value, 1)


def set_private_transport_feature_id(feature, id_):
    """Set the id of this feature"""
    feature.WriteKey(PRIVATE_TRANSPORT_FEATURE_ATTRS['id'], id_, 1)


def set_private_transport_feature_type(feature, type_):
    """Set the type of feature this object is.

    Args:
        feature
        type_ (str)
    """
    feature.WriteKey(PRIVATE_TRANSPORT_FEATURE_ATTRS['type'], type_, 1)


def is_private_transport_feature(obj):
    """Return whether an object is a private transport feature"""
    return obj.ReadKey(PRIVATE_TRANSPORT_FEATURE_ATTRS['flag']) == '1'


def generate_feature_id():
    """Generate a unique ID for a feature

    Returns:
        (str) A unique ID for a private ship feature. This is guaranteed
        to be unique across all private ship features.
    """
    return idgen.generate()
