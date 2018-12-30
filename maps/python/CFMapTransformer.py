# CFMapTransformer.py - CFMapTransformer class
#
# Copyright (C) 2007 David Delbecq
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# The author can be reached via e-mail at tchize+cfpython@gmail.com
#
# Small helper class. Instanciate with
# transform = CFMapTransformer(key)
# or
# transform = CFMapTransformer(key, map)
# default map value is to use The map of current event.
#
# The key is used to keep track of transform. Different transforms
# on same map should use different key to prevent mixup

# transformAll(criteria,whatTo)
# criteria can be a single String or a list,
# whatTo can be a single String or a list.
# This will scan the whole map for item's whose
# "name" are in the list of criteria and transform
# the to one of the whatTo objects, choosen randomly.
# original object is kept in inventory of newly created
# item and can be restored from there
# untransformAll()
# this no argument method cancel all changes that have been
# made by transformAll
import Crossfire
import random


def GetObjectMap(o):
    while (o.Env):
        o=o.Env
    if (o.Map):
        return o.Map
    return None

def MakeIdentifier(key):
    m = Crossfire.CreateObjectByName("force")
    m.Name= key
    m.Speed=0
    return m

class CFMapTransformer:
    key = None
    cfmap = None

    def __init__(self,key,cfmap=None):
        self.key=key
        if (cfmap):
            self.cfmap=cfmap
        else:
            o = Crossfire.WhoAmI()
            while (o.Env):
                o = o.Env
            self.cfmap = o.Map

    def transformAll(self, criteria, whatTo):
        if (not isinstance (criteria,list)):
            criteria = [criteria]
        for x in range (self.cfmap.Width):
            for y in range (self.cfmap.Height):
                top = self.cfmap.ObjectAt(x,y)
                while (top):
                    #print "testing %s at (%d %d) for match against criteria %s" %(top.Name,x,y,criteria)
                    next = top.Above
                    if (set([top.Name]) & set (criteria) ):
                        #print "matched"
                        #do the replace
                        if isinstance(whatTo,list):
                            ob = Crossfire.CreateObjectByName(whatTo[random.randint(0,len(whatTo)-1)])
                        else:
                            ob = Crossfire.CreateObjectByName(whatTo)
                        force = MakeIdentifier(self.key)
                        force.InsertInto(ob)
                        top.InsertInto(ob)
                        ob.Pickable=False
                        self.cfmap.Insert(ob,x,y)
                        #handle living stuff by freezing them
                        force.WriteKey("inside_speed","%f" %top.Speed,1)
                        top.Speed = 0
                    top=next

    def untransformAll(self):
        for x in range(self.cfmap.Width):
            for y in range(self.cfmap.Height):
                top = self.cfmap.ObjectAt(x,y)
                while (top):
                    #print "checking at (%d,%d) if %s need a restore" %(x,y,top.Name)
                    next = top.Above
                    match = False
                    inv = top.Inventory
                    torestore = None
                    while(inv):
                        #print "    checking inventory item %s" %inv.Name
                        if (inv.Type == Crossfire.Type.FORCE) and (inv.Name == self.key):
                            oldspeed = float(inv.ReadKey("inside_speed"))
                            #print "i found the force, luke"
                            match = True
                        elif (inv.Type != Crossfire.Type.EVENT_CONNECTOR):
                            #print "found what to restore"
                            torestore = inv
                        inv = inv.Below
                    if match and (torestore != None):
                        #print "found something to restore"
                        torestore.Speed = oldspeed
                        self.cfmap.Insert(torestore,x,y)
                        top.Remove()
                    top=next
