"""
Implementation of the system that tracks objects

Each object is assumed to have 0 or more trackers on top of it. Objects
with more than 1 trackers (or 1 tracker) are tracked. Tracked objects
can be retrieved using the get_object() function.

Interface:

    add_tracker(obj): Add a tracker to the obj and maybe update obj
        location.
    remove_tracker(tracking_id): Remove tracker from an object, given
        its tracking ID.
    get_object(tracking_id): Retrieve tracked object, given its tracking
        ID.

    Use it only with unique objects that cannot be picked up.

    Behavior is undefined for pickable objects.

Inner workings:

    While in game, objects can exist in three states, basically:

    - Live on a loaded map
    - Live on an unloaded map
    - Removed

    We also assign a tracking ID to each object we track.

    This system keeps track of objects, no matter what state they are
    in.

    Objects on a loaded map are saved in a dictionary (we call them
    references).

    Objects on an unloaded map are saved in a database (we call the data
    saved coordinates).

    Removed objects are marked as lost, for any user that want to search
    for it later know that we lost it.

    We capture system events to recognize when an object moves from one
    state to another, and update our tracking method accordingly.

Hooks:

    There are 4 types of events that can be fired related to Crossfire maps:

    - MAPLOAD: After a map is fully loaded, including objects on
      the map (I had to change this a bit in the server code).
    - FREE_OBJECTS: I created this event to signal me that all objects
      are gonna be removed from a map.
    - MAPRESET: Before resetting a map. Resetting a map will reset all
      its objects except for the unique ones (and other unique-related
      stuff (BTW, unique would be better named 'permanent').
    - MAPUNLOAD: Before swapping out a map. The objects will still
      be present there, except when it is the result of a RESET.
    - SERVER_SHUTDOWN: I also created this event to signal that the
      server is gonna be shut down. The MAPRESET and MAPUNLOAD were not
      firing then, but I haven't tested FREE_OBJECTS.

    The tracking system responds in the following way to each of these
    events (converting only the necessary objects):

    - load process:
      - MAPLOAD -> coords2ref

    - unload process:
      - MAPUNLOAD -> ref2coords
      - FREE_OBJECTS -> obj saved as coords, not affected

    - reset process (the reason FREE_OBJECTS is needed):
      - FREE_OBJECTS -> ref2coords
      - MAPUNLOAD -> obj saved as coords, NOP
      - FREE_OBJECTS -> obj saved as coords, NOP
      - MAPRESET -> obj saved as coords, NOP

    I left a hook for MAPUNLOAD, MAPRESET and SERVER_SHUTDOWN as backup
    measure and because this won't hurt.

Notes:

    This code is intricate and has not been tested thoroughly, so, there
    will certainly be errors.

    I hope that my non-automated tests are enough to make it robust
    enough.

    It currently doesn't have a failsafe against server crashes or loss
    of power, so, objects saved as refs when the server crashes will be
    lost.

    Also, thinking of this whole code as a finite state machine may be
    useful. Actions can be made by the system or by the server/players
    themselves. I was thinking of providing an image of this but I got
    a bit lazy lol

    This code could probably be easier implemeted (and be more useful)
    if done in C. I don't know C, so, that's all you get :D
"""

import os.path
import sqlite3

import Crossfire

from utils import iter_loc
from utils import log_error


DATABASE_NAME = 'tracking_system.db'

FIELD_NAME_TRACKING_ID = 'tracking_id'

TRACKING_METHOD_REF = 'by_ref'
TRACKING_METHOD_COORDS = 'by_coord'
TRACKING_METHOD_NONE = 'not_tracked'

REFS_DICTIONARY_NAME = 'tracking_system_refs'


class InvalidObjectError(Exception):
    """When an object is invalid and this impossibilitate the execution
    of the function you expected"""


class ObjectNotTrackedError(Exception):
    """When you try to retrieve an object reference, but it is not
    tracked, we will issue an error"""


class SubstitutingTrackingIdError(Exception):
    """When you try to write the tracking id to an object that already
    has it"""


# Interface

def add_tracker(object_):
    """Add a tracker for object_ and return the object tracking ID"""

    if not _is_object_valid(object_):
        raise InvalidObjectError

    tracking_id = _get_tracking_id(object_)
    if tracking_id:
        _increment_number_of_trackers(tracking_id)
        _set_object_reference(tracking_id, object_)
    else:
        tracking_id = _generate_new_tracking_id()
        _set_tracking_id(object_, tracking_id)
        _increment_number_of_trackers(tracking_id)
        _set_object_reference(tracking_id, object_)

    return tracking_id


def remove_tracker(tracking_id):
    """Remove a tracker from an object, given its tracking_id"""

    assert isinstance(tracking_id, str)
    assert tracking_id.isdigit()

    num_trackers = _decrement_number_of_trackers(tracking_id)
    if num_trackers == 0:
        _stop_tracking(tracking_id)


def get_object(tracking_id, raise_on_no_trackers=False):
    """Return a reference to an object, given its tracking_id

    If the object cannot be found, return None.

    If the object was not being tracked, raise an error when
    raise_on_no_trackers = True. This is indicative of programming
    error, because the number of trackers for an item shall remain
    consistent.
    """

    assert isinstance(tracking_id, str)
    assert tracking_id.isdigit()

    if _get_number_of_trackers(tracking_id) == 0 and raise_on_no_trackers:
        raise ObjectNotTrackedError

    # get reference to obj or None
    tracking_method = _get_tracking_method(tracking_id)
    if tracking_method == TRACKING_METHOD_REF:
        obj = _get_object_by_reference(tracking_id)
    elif tracking_method == TRACKING_METHOD_COORDS:
        obj = _get_object_by_coords(tracking_id)
    elif tracking_method == TRACKING_METHOD_NONE:
        obj = None
    else:
        raise RuntimeError('should not reach this line of code')

    # mark object as lost if I can't find it
    if obj is None and tracking_method != TRACKING_METHOD_NONE:
        _mark_object_as_lost(tracking_id)

    return obj


# Inner workings

def _is_object_valid(object_):
    """Return True if an object is valid"""
    # object is invalid if it is None
    if object_ is None:
        return False

    # object is invalid if it has been freed
    try:
        object_.Removed
    except ReferenceError:
        return False

    # object is valid if it has not been removed
    return not object_.Removed


def _get_tracking_id(object_):
    """Return the tracking id of an object, or '', if it has none

    The object is assumed to be valid.

    Raises:
        ReferenceError: if the object is freed.

    Returns:
        An str corresponding to the tracking id for the object.
    """
    return object_.ReadKey(FIELD_NAME_TRACKING_ID)


def _set_tracking_id(object_, tracking_id):
    """Set tracking id for an object.

    The object is assumed to be valid.

    Raises:
        ReferenceError: if the object is freed.
        SustitutingTrackingIdError: if the object already has a tracking
            id.

    Returns:
        Nothing.
    """
    # check if the object already has a tracking id
    # if it has, throw an Error
    cur_id = _get_tracking_id(object_)
    if cur_id:
        raise SubstitutingTrackingIdError

    # if it doesn't have, set it !
    object_.WriteKey(FIELD_NAME_TRACKING_ID, tracking_id, 1)


def _delete_tracking_id(object_):
    """Delete the tracking id of an object.

    The object is assumed to be valid.
    """
    # check if object has tracking id
    # if not, warn the user and exit
    cur_id = _get_tracking_id(object_)
    if not cur_id:
        _warn('Trying to remove tracking id from object without tracking id')
        return
    # delete tracking id
    object_.WriteKey(FIELD_NAME_TRACKING_ID, '', 1)


def _get_number_of_trackers(tracking_id):
    """Return the number of trackers tracking an object"""
    conn = _get_db_connection()
    with conn:
        # get row corresponding to tracking_id
        cur = conn.cursor()
        cur.execute(
            'select num_trackers from tracked_objects where id=?',
            (int(tracking_id),),
        )
        result = cur.fetchone()
    conn.close()

    # if no rows, return 0 (object is not tracked)
    if result is None:
        return 0

    return result[0]


def _increment_number_of_trackers(tracking_id):
    """Increment number of trackers tracking an object

    Returns:
        (int) New number of trackers on the object.
    """
    conn = _get_db_connection()
    with conn:
        # get row corresponding to tracking_id
        cur = conn.cursor()
        cur.execute(
            'select num_trackers from tracked_objects where id=?',
            (int(tracking_id),),
        )
        result = cur.fetchone()

        # if no row, create row
        if result is None:
            num_trackers = 1
            cur.execute(
                'insert into tracked_objects (id, num_trackers) values (?, ?)',
                (int(tracking_id), num_trackers),
            )
        # if row, increment number of trackers
        else:
            num_trackers = result[0] + 1
            cur.execute(
                'update tracked_objects set num_trackers=? where id=?',
                (num_trackers, int(tracking_id)),
            )

    conn.close()
    return num_trackers


def _decrement_number_of_trackers(tracking_id):
    """Decrement number of trackers tracking an object

    If the number is already 0, keep it at zero.

    Returns:
        (int) New number of trackers on the object.
    """
    conn = _get_db_connection()
    with conn:
        # get row corresponding to tracking_id
        cur = conn.cursor()
        cur.execute(
            'select num_trackers from tracked_objects where id=?',
            (int(tracking_id),),
        )
        result = cur.fetchone()

        # if no row, warn user and exit
        if result is None:
            _warn('Trying to remove a tracker from an object that is not saved on the db.')
            return 0

        # if no trackers already, warn user and exit
        if result[0] == 0:
            _warn('Trying to remove a tracker from an object without trackers.')
            return 0

        # decrement number of trackers
        num_trackers = result[0] - 1
        cur.execute(
            'update tracked_objects set num_trackers=? where id=?',
            (num_trackers, int(tracking_id)),
        )

    conn.close()
    return num_trackers


def _get_tracking_method(tracking_id):
    """Return the current tracking method used to track an object

    Returns:
        One of TRACKING_METHOD_, defined as contants
    """

    conn = _get_db_connection()
    with conn:
        cur = conn.cursor()
        cur.execute(
            'select method from tracked_objects where id=?',
            (int(tracking_id),),
        )
        result = cur.fetchone()
    conn.close()

    if result is None:
        return TRACKING_METHOD_NONE

    return result[0]


def _stop_tracking(tracking_id):
    """Stop tracking an object"""

    # remove tracking id from object (if found)
    object_ = get_object(tracking_id, raise_on_no_trackers=False)
    if object_ is not None:
        _delete_tracking_id(object_)

    # delete object reference
    _delete_object_reference(tracking_id)

    # clean object database row
    conn = _get_db_connection()
    with conn:
        conn.execute(
            'delete from tracked_objects where id=?',
            (int(tracking_id),),
        )
    conn.close()


def _set_object_reference(tracking_id, object_):
    """Update the reference of an object and set its tracking method to
    REF.

    Raises:
        InvalidObjectError: if object_ is invalid.
    """
    # check if object is valid
    if not _is_object_valid(object_):
        raise InvalidObjectError
    # update reference
    _get_objects_refs_dictionary()[tracking_id] = object_
    # update tracking method to REF
    conn = _get_db_connection()
    with conn:
        conn.execute(
            'UPDATE tracked_objects '
            'SET method=? '
            'WHERE id=?',
            (TRACKING_METHOD_REF, tracking_id),
        )
    conn.close()


def _delete_object_reference(tracking_id):
    """Remove the reference to an object"""
    try:
        del _get_objects_refs_dictionary()[tracking_id]
    except KeyError:
        _warn('Trying to remove reference for an object without reference saved')
        return


def _generate_new_tracking_id():
    """Generate a new tracking ID

    This will have the side effect of creating a database row and we use
    it later.

    Returns:
        (str) A new tracking ID.
    """
    conn = _get_db_connection()
    with conn:
        cur = conn.cursor()
        cur.execute(
            'insert into tracked_objects (id, method, num_trackers) values (NULL,?,?)',
            (TRACKING_METHOD_NONE, 0),
        )
        id_ = str(cur.lastrowid)
    conn.close()
    return id_


def _get_object_by_reference(tracking_id):
    """Return object whose reference has been saved

    If object is not found, or if it is invalid, return None.
    """
    obj = _get_objects_refs_dictionary().get(tracking_id)
    return obj if obj is not None and _is_object_valid(obj) else None


def _get_object_by_coords(tracking_id):
    """Get an object by saved coordinates

    This will retrieve saved coordinates, and, if the map is not already
    ready, instantiate the map and get a reference to the object.

    The reference will be returned.

    If the object cannot be found, return None.

    Returns:
        A reference to the object tracked by tracking_id, or None.
    """
    conn = _get_db_connection()
    with conn:
        cur = conn.cursor()
        cur.execute(
            'select mapname, coord_x, coord_y from tracked_objects where id=?',
            (int(tracking_id),),
        )
        result = cur.fetchone()
    conn.close()

    if not result:
        return None

    # extract coordinates and map name
    mapname, coord_x, coord_y = result

    # instantiate map
    map_ = Crossfire.ReadyMap(mapname)
    # if not possible, return None
    if map_ is None:
        _warn("Could not ready map %s for object with tracking id %s" % (mapname, tracking_id))
        return None

    # get reference to object
    # (and return)
    return _get_object_in_map(tracking_id, map_, coord_x, coord_y)


def _mark_object_as_lost(tracking_id):
    """We lost the reference to a certain object, mark it as lost"""
    # mark it as lost in the database
    conn = _get_db_connection()
    conn.execute(
        'update tracked_objects set method=? where id=?',
        (TRACKING_METHOD_NONE, int(tracking_id)),
    )
    conn.commit()
    conn.close()


def _is_object_on_map(object_, map_):
    """Return true if object_ still exists and is on map_"""
    o = object_
    m = map_
    return object_.Exists and not object_.Removed and object_.Map == map_


def _get_object_in_map(tracking_id, map_, coord_x, coord_y):
    """Return a reference to the object with tracking_id sitting at map_

    If the object cannot be found at coord_x / coord_y, return None.

    Args:
        tracking_id: The tracking_id of the object to be found.
        map_: The map (already readied) where the object will be
            searched for.
        coord_x: the X coordinate we expect the item to be in.
        coord_y: The Y coordinate we expect the item to be in.

    Returns:
        A reference to the object in the map, or None, if the object was
        not found.
    """

    assert isinstance(tracking_id, str)

    for obj in iter_loc(map_, coord_x, coord_y):
        if _get_tracking_id(obj) == tracking_id:
            return obj
    # obj was not found
    return None


def _ref_to_coords(tracking_id):
    """Convert tracking method of an object from REF to COORDS"""

    object_ = _get_object_by_reference(tracking_id)

    # remove REF
    _delete_object_reference(tracking_id)

    # add coords (uses obj)
    mapname = object_.Map.Path
    coord_x = object_.X
    coord_y = object_.Y
    conn = _get_db_connection()
    with conn:
        conn.execute(
            'UPDATE tracked_objects '
            'SET method=?, mapname=?, coord_x=?, coord_y=? '
            'WHERE id=?',
            (TRACKING_METHOD_COORDS, mapname, coord_x, coord_y, tracking_id),
        )
    conn.close()


def _coords_to_ref(tracking_id, object_):
    """Convert tracking method of an object from COORDS to REF"""

    # add ref
    _set_object_reference(tracking_id, object_)

    # remove coords
    conn = _get_db_connection()
    with conn:
        conn.execute(
            'UPDATE tracked_objects '
            'SET method=? '
            'WHERE id=?',
            (TRACKING_METHOD_REF, tracking_id),
        )
    conn.close()


def _get_db_connection():
    """Return sqlite3.Connection for the database we are using

    This will also initialize the required tables for use.
    """

    # get location of db
    loc = _get_db_location()

    # initialize connection object
    conn = sqlite3.connect(loc)

    # create table if they don't exist
    # (trying to create the table every time is inefficient but fast enough)
    conn.execute('''
        create table if not exists tracked_objects (
            id integer primary key autoincrement,
            method text,
            num_trackers integer,
            mapname text,
            coord_x integer,
            coord_y integer
        )
    ''')

    return conn


def _get_db_location():
    """Return the location for the sqlite3 database we will be using"""
    return os.path.join(
        Crossfire.LocalDirectory(),
        DATABASE_NAME,
    )


def _get_objects_refs_dictionary():
    # this script is gonna be imported by another, therefore, we need to
    # use the shared dict
    private_d = Crossfire.GetSharedDictionary()
    # create dictionary if it doesn't exist
    if private_d.get(REFS_DICTIONARY_NAME) is None:
        private_d[REFS_DICTIONARY_NAME] = {}
    return private_d[REFS_DICTIONARY_NAME]


# Hooks

def hook_freeobjects(map_):
    """Hook called on EVENT_FREE_OBJECTS

    Objects are being removed from a map. Convert all tracked objects
    there into coordinates.

    Good references are converted into map coordinates and saved to the
    database.

    Bad references are dropped out and we mark the object as lost.
    """
    for tracking_id, object_ in _get_objects_refs_dictionary().items():
        if _is_object_on_map(object_, map_):
            if _is_object_valid(object_):
                _ref_to_coords(tracking_id)
            else:
                _mark_object_as_lost(tracking_id)


def hook_mapload(map_):
    """Hook called on EVENT_MAPLOAD

    A map is loading. Convert all tracked objects in this map to
    references.

    Good coordinates are converted into object references and saved to
    the refs dictionary.

    Bad coordinates are dropped out and we mark the object as lost.

    Note: The event should (and does) come after the objects have been
    loaded.
    """

    mapname = map_.Path

    # get objects on map
    conn = _get_db_connection()
    with conn:
        cur = conn.cursor()
        cur.execute(
            'SELECT id, coord_x, coord_y '
            'FROM tracked_objects '
            'WHERE mapname = ? AND method = ? AND num_trackers >= 1',
            (mapname, TRACKING_METHOD_COORDS),
        )
        results = cur.fetchall()
    conn.close()

    # iterate over objects
    for tracking_id, coord_x, coord_y in results:
        tracking_id = str(tracking_id)
        object_ = _get_object_in_map(tracking_id, map_, coord_x, coord_y)
        if object_ is not None:
            _coords_to_ref(tracking_id, object_)
        else:
            _mark_object_as_lost(tracking_id)


# Utils

def _warn(msg):
    log_error("Tracking system: %s" % msg)
