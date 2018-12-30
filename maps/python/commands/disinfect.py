import Crossfire

whoami = Crossfire.WhoAmI()
map = whoami.Map

if map != None and whoami.DungeonMaster:
    count = 0
    map.Print('%s disinfecting %s'%(whoami.Name, map.Path))
    for x in range(0, map.Width):
        for y in range(0, map.Height):
            o = map.ObjectAt(x, y)
            while o != None:
                a = o.Above
                if o.Type == Crossfire.Type.DISEASE:
                    whoami.Message(' removing %s at %d,%d'%(o.Name, x, y))
                    count = count + 1
                    o.Remove()
                o = a
    if count == 0:
        map.Print(' => nothing removed')
    else:
        map.Print(' => %d diseases removed'%count)