#CFWorld.py
#A small module for checking where is bigworld an object is if it's in bigworld.
import Crossfire

world_prefix = "/world/world_"
world_prefix_len = len(world_prefix)
world_len = len(world_prefix) + len('xxx_xxx')
world_sep = '_'
bigmapxsize = 50
bigmapysize = 50

#Return an x,y tuple of where in bigworld an object is. Return false if not in bigworld. In current bigworld, values range from 5000 to 6499.
def loc_from_ob(ob):
    cfmap = ob.Map
    if ((cfmap.Path.find(world_prefix) != 0) or (len(cfmap.Path) != world_len)):
        return False
    strloc = cfmap.Path[world_prefix_len:].split(world_sep)
    x = (int(strloc[0]) * bigmapxsize) + ob.X
    y = (int(strloc[1]) * bigmapysize) + ob.Y
    return (x, y)
