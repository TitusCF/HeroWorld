import Crossfire
import os

whoami = Crossfire.WhoAmI()
whoisother = Crossfire.WhoIsOther()
parms = Crossfire.WhatIsMessage()

if parms=="give":
    Crossfire.Log(Crossfire.LogDebug, "Object given: %s" %(whoisother.Name))
    key = whoisother.ReadKey("mad_mage_marker")
    Crossfire.Log(Crossfire.LogDebug, "Object key  : %s" %(key))
    if key:
        mypath = os.path.join(Crossfire.DataDirectory(), Crossfire.MapDirectory(), 'python/maps/scorn/towers/mad_mage',key)
        Crossfire.Log(Crossfire.LogDebug, "Object path : %s" %(mypath))
        file = open(mypath, "r")
        for line in file:
            whoami.Say(line)
        file.close()
        if (key == 'brewery_letter'):
            whoisother.Name = "Milking machine blueprints"
            whoisother.WriteKey("mad_mage_marker", "milking_blueprints")
    else:
        whoami.Say("Yes?")
