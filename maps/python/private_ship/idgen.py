"""
ID generator for private transports and features they can have
"""

import os.path
import sqlite3

import Crossfire


def generate():
    """Generate a unique id for a private ship or a private ship
    feature.

    I will be using a database here, first, because it is easy to setup,
    and second, because it deals with race conditions in a correct
    manner. Implementing this is possible, but prone to errors.

    Returns:
        An ID (str) to be used with private ships and private ships
        features.
    """

    # create database if it doesn't exist
    conn = sqlite3.connect(get_db_location())

    c = conn.cursor()

    # create table if it doesn't exist
    c.execute('create table if not exists id_generator (id integer primary key autoincrement)')

    # insert new entry into table
    c.execute('insert into id_generator values (NULL)')

    # commit transaction
    conn.commit()

    # retrieve id of generated entry and convert to str
    id_ = str(c.lastrowid)

    # return id
    return id_


def get_db_location():
    """Get path of database file we will use here"""
    return os.path.join(
        Crossfire.LocalDirectory(),
        'private_transports_id_gen.db',
    )
