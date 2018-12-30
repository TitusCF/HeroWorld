import Crossfire

def getReceiver(me, direction):
    map = me.Map
    x = me.X
    y = me.Y

    if direction==2:
        ob = map.ObjectAt(x+1, y-1)
    elif direction==3:
        ob = map.ObjectAt(x+1, y)
    elif direction==4:
        ob = map.ObjectAt(x+1, y+1)
    elif direction==5:
        ob = map.ObjectAt(x, y+1)
    elif direction==6:
        ob = map.ObjectAt(x-1, y+1)
    elif direction==7:
        ob = map.ObjectAt(x-1, y)
    elif direction==8:
        ob = map.ObjectAt(x-1, y-1)
    else:
        ob = map.ObjectAt(x, y-1)
    return ob

whoami = Crossfire.WhoAmI()
name = Crossfire.ScriptName()
parms = Crossfire.ScriptParameters()

if not parms:
    whoami.Message("Show which object ?")
else:
    op = whoami.CheckInventory(parms)
    if not op:
        whoami.Message('No matching object found to give.')
    else:
        direction = whoami.Facing
        receiver = getReceiver(whoami, direction)
        if not receiver:
            whoami.Message('Nobody to give this to.')
        else:
            top = receiver
            while(top):
                next = top.Above
                top.Event(whoami,op, "give", 0)
                top = next
