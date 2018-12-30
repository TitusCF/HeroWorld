import Crossfire
import CFWorld
import math

scorn_loc = (5272, 5786)
navar_loc = (6112, 5850)

Crossfire.SetReturnValue( 1 )

def getdiff(loc1, loc2):
    return (loc1[0]-loc2[0], loc1[1]-loc2[1])

def getdir(loc1, loc2):
    loc = getdiff(loc1, loc2)
    if (loc[1] > 0):
        start="south"
    elif(loc[1] < 0):
        start="north"
    else:
        start=""
    if (loc[0] > 0):
        return start+"east"
    elif(loc[0] < 0):
        return start+"west"
    else:
        return start

#outputs in furlongs (outdoor tiles)
def getdist(loc1, loc2):
    loc = getdiff(loc1, loc2)
    return int(math.sqrt((loc[0]*loc[0])+(loc[1]*loc[1])))

#outputs in miles
def getuserdist(dist):
    return (int(dist/8.0+0.5))

def gettext(loc1, loc2, name):
    loc_raw_dist = getdist(loc1, loc2)
    loc_dist = getuserdist(loc_raw_dist)
    loc_dir = getdir(loc2, loc1)
    if (abs(loc_dist) > 5):
        loc_distmsg = "A "+name+" arrow flashes "+str(loc_dist)+" times"
    else:
        loc_distmsg = "A "+name+" arrow glows steady"
    if (loc_raw_dist):
        loc_distmsg += ", pointing to the "+loc_dir+"."
    else:
        loc_distmsg += ", spinning in one place."
    return loc_distmsg

pl = Crossfire.WhoIsActivator()
me = Crossfire.WhoAmI()

location = CFWorld.loc_from_ob(pl)
if (location):
    scorntxt = gettext(location, scorn_loc, "red")
    navartxt = gettext(location, navar_loc, "blue")
    pl.Write(scorntxt+" "+navartxt)
else:
    pl.Write("The amulet doesn't seem to work here.")
