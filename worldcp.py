from shutil import copyfile
import os
import sys

WORLD_BASE = 100
WORLD_SIZE = 30

path = sys.argv[1]
for x in range(WORLD_SIZE):
    for y in range(WORLD_SIZE):
        dst = "%s_%d_%d" % (os.path.basename(path), x + WORLD_BASE, y + WORLD_BASE)
        copyfile(path, dst)
