#!/usr/bin/env python3
help_text = """
retile.py -- retile Crossfire world maps (titus edition)
usage: python3 retile.py WORLD ...

NOTE: This only works when the current directory is maps/world!
There are some configurable options at the top of the script.
""".strip()

ENABLE_UPDOWN = True
WORLD_BASE = 100
WORLD_SIZE = 30

import multiprocessing
import re
import sys

def sign(n:int):
    if n > 0:
        return 1
    elif n < 0:
        return -1
    else:
        return 0

def ground_wrap_dim(nm, nx):
    if nx > WORLD_BASE + WORLD_SIZE-1 or nx < WORLD_BASE:
        if nm == 'world':
            nm = 'bottomworld'
            nx = nx + WORLD_SIZE*-sign(nx - WORLD_BASE)
        elif nm == 'bottomworld':
            nm = 'world'
            nx = nx + WORLD_SIZE*-sign(nx - WORLD_BASE)
    return (nm, nx)

def wrap_dim(nx):
    return (nx - WORLD_BASE) % WORLD_SIZE + WORLD_BASE

def wrap_tile(m):
    nm = m[0]
    nl = m[1]
    nx = m[2]
    ny = m[3]
    if nl == 0:
        nm, nx = ground_wrap_dim(nm, nx)
        nm, ny = ground_wrap_dim(nm, ny)
    else:
        nx = wrap_dim(nx)
        ny = wrap_dim(ny)
    return (nm, nl, nx, ny)

def tile_map(m):
    n, l, x, y = m
    nesw = list(map(wrap_tile, [
        (n, l, x, y - 1), # N
        (n, l, x + 1, y), # E
        (n, l, x, y + 1), # S
        (n, l, x - 1, y)  # W
    ]))

    if ENABLE_UPDOWN and n == 'bottomworld':
        up_down = list(map(wrap_tile, [
            (n, l+1, x, y),   # Up
            (n, l-1, x, y)    # Down
        ]))
        return nesw + up_down
    else:
        return nesw

def show_tiling(ts):
    s = []
    for n, t in enumerate(ts, 1):
        s.append("tile_path_%d %s\n" % (n, t))
    return s

def read_mapname(name:str):
    matches = re.findall("(.*)_(\d+)_(\d+)", name)
    try:
        res = matches[0]
        name = res[0]
        x = int(res[1])
        y = int(res[2])
        level = re.findall("(.*)_(-?\d+)", name)
        if len(level) == 0:
            return (name, 0, x, y)
        else:
            return (level[0][0], int(level[0][1]), x, y)
    except:
        return None

def show_mapname(m):
    if m[1] == 0:
        return "%s_%d_%d" % (m[0], m[2], m[3])
    else:
        return "%s_%d_%d_%d" % m

def isTile(s):
    return s.startswith("tile_path_")

def indexOfFirst(pred, xs):
    for n, x in enumerate(xs):
        if pred(x):
            return n
    return -1

def splice(l, newl, index):
    return l[:index] + newl + l[index:]

def mapfile_read(path):
    with open(path) as f:
        return f.readlines()

def mapfile_write(path, m):
    with open(path, "w") as f:
        f.write("".join(m))

def mapfile_replace_tiles(m, tiles):
    index = indexOfFirst(isTile, m)
    m = list(filter(lambda x: not isTile(x), m))
    return splice(m, tiles, index)

def retile_map(path, mf):
    m = read_mapname(path)
    new_tiles = show_tiling(map(show_mapname, tile_map(m)))
    return mapfile_replace_tiles(mf, new_tiles)

def process_map(path):
    mf = mapfile_read(path)
    nf = retile_map(path, mf)
    if nf != mf:
        mapfile_write(path, nf)

def main():
    if len(sys.argv) <= 1:
        print(help_text)
        return 1
    p = multiprocessing.Pool()
    p.map(process_map, sys.argv[1:])

if __name__ == '__main__':
    main()
