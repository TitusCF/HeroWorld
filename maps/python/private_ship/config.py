"""
Configuration for the private ship scripts !
"""

import Crossfire

import private_ship.path as path


ERROR_MESSAGE_FLAGS = Crossfire.MessageFlag.NDI_UNIQUE
INFORMATION_MESSAGE_FLAGS = Crossfire.MessageFlag.NDI_UNIQUE


# Messages for the player
# DO NOT MODIFY THE KEY of the messages (but the content is okay)
PLAYER_MESSAGES = {

    # Error Messages

    'action_error_basic': "Sorry, cannot do that.",
    'internal_error': "Sorry, cannot do that. Contact a DM if you keep having this problem.",
    'scroll_not_renamed': "You must rename the scroll before applying it to the ship.",
    'player_not_inside_transport': "Cannot do this outside of a transport.",
    'player_inside_public_transport': "Use a personal transport!",
    'not_owner_of_transport': "You are not owner of this ship!",
    'transport_has_no_owner': "You are not owner of this ship!",
    'transport_owned_by_someone_else': "You are not the owner of this ship!",
    'cannot_create_more_cabins': "Sorry, you can't add more cabins to this ship.",
    'player_cannot_use_feature': "You must be the owned of this ship or have a key, to try that.",
    'lost_track_of_transport': "Sorry, we lost track of your transport. We will be teleporting you to your bed, instead.",
    'can_only_have_one_anchor': "Sorry, you cannot add more anchors to this ship.",

    # Success Messages

    'transport_renamed': "Congratulations! You have renamed this ship to {transport_name}.",
    'transport_is_mine': "You are now the owner of {transport_name}.",
    'cabin_created': "You have added a cabin to your ship!",
    'anchor_added': "You added an anchor to the ship.",

    # Status
    'transport_anchor_activated': "Dropping the anchor!",
    'transport_anchor_deactivated': "Raising the anchor. Full speed ahead!",
}


# Attributes found in a private transport
PRIVATE_TRANSPORT_ATTRS = {

    # Indicates whether I am a private transport or not
    #   If this attribute is missing, it means I'm not a private transport!
    'flag': 'private_transport',

    # Indicates the name of my owner
    #   If this attribute is missing, it mean I still don't have an owner
    'owner_name': 'private_transport_owner',

    # Indicates the maximum amount of cabins that fit this transport
    'limit_cabins': 'private_transport_cabins_limit',

    # The value of this attribute indicates which template I will use
    # when creating a new cabin.
    'cabin_template': 'private_transport_cabin_template',
    'cabin_entrance_x': 'private_transport_cabin_entrance_x',
    'cabin_entrance_y': 'private_transport_cabin_entrance_y',
    'cabin_exit_x': 'private_transport_cabin_exit_x',
    'cabin_exit_y': 'private_transport_cabin_exit_y',

    # Anchor stuff
    'anchored': 'private_transport_anchored',
    # The value of this attribute indicates the speed the transport usually moves
    # Set as the same as speed, if don't want weird behaviour
    'base_speed': 'private_transport_base_speed',
}


# Attributes found in a private transport feature
PRIVATE_TRANSPORT_FEATURE_ATTRS = {
    'flag': 'private_transport_feature',
    'type': 'private_transport_feature_type',
    'id': 'private_transport_feature_id',
}


NEW_TRANSPORT_NAME = '{scroll_name} (owner - {player_name})'


# Cabins

CABINS_DIRECTORY = path.map2abs('/world_built/cabins')

# Note: Default value for the limit of cabins for a private transport
# when PRIVATE_TRANSPORT_ATTRS['limit_cabins'] (see above) is not set on
# the transport. You don't need worry about this.
CABIN_LIMIT_DEFAULT = 4

CABIN_KEY_ARCHETYPE = 'cabin_key'
CABIN_KEY_NAME = 'key to Cabin {id_}'
CABIN_KEY_NAME_PL = 'keys to Cabin {id_}'
CABIN_KEY_GENERATED_COUNT = 5

EXTERNAL_CABIN_DOOR_ARCHETYPE = 'external_cabin_door'
EXTERNAL_CABIN_DOOR_NAME = 'Cabin {id_}'

INTERNAL_CABIN_DOOR_ARCHETYPE = 'internal_cabin_door'
INTERNAL_CABIN_DOOR_ATTR_TRACKING_ID = 'private_transport_tracking_id'


# Anchors

ANCHOR_ARCHETYPE = 'anchor'
ANCHOR_NAME = 'Anchor {id_}'

ANCHOR_KEY_ARCHETYPE = 'anchor_key'
ANCHOR_KEY_NAME = 'key to Anchor {id_}'
ANCHOR_KEY_NAME_PL = 'keys to Anchor {id_}'
ANCHOR_KEY_GENERATED_COUNT = 5


# Feature keys

FEATURE_KEY_SLAYING_PREFIX = 'private_transport_feature_id_'
