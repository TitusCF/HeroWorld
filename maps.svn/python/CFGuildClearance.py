import Crossfire
import CFGuilds

import sys
import string
activator=Crossfire.WhoIsActivator()
activatorname=activator.Name
mymap = activator.Map
def find_player(object):
    while (object.Type != 1) : #1 is type 'Player'
        object = object.Above
        if not object:
            return 0
    return object




whoami=Crossfire.WhoAmI()

def CheckClearance(lParams,oActivator):
        
        ClearanceLevels={"Initiate":1,"Novice":2,"Guildman":3,"Journeyman":4,"Master":5,"GuildMaster":6}
        oGuild=CFGuilds.CFGuild(lParams[0])
        
        if oActivator.DungeonMaster==1:
                return True
        iClearanceLevel=ClearanceLevels.get(lParams[1],0)
        dGuildInfo=oGuild.info(oActivator.Name)
        
        if dGuildInfo==0:
            return False

        iClearanceApproved=ClearanceLevels.get(dGuildInfo['Rank'],0)
        if dGuildInfo["Status"]=='suspended':
                iClearanceApproved=0
                Crossfire.WhoIsActivator().Say("You are currently suspended from the guild.")
        elif dGuildInfo["Status"]=="probation":
                Crossfire.WhoIsActivator().Say("You are currently on probation.")
        if iClearanceLevel>iClearanceApproved:
                return False
        return True
        

if __name__=='__builtin__':

        texta = [ '' ]
        if Crossfire.WhatIsMessage():
            texta=string.split(Crossfire.WhatIsMessage())

        if (texta[0].upper() == 'ENTER'):
                Params=string.split(Crossfire.ScriptParameters())
                if CheckClearance(Params,activator):
                        Approved = "Access granted"
                else:
                       Approved = "Access denied"
                whoami.Say(Approved)
                

                if (Approved == 'Access granted'):
                        mymap.TriggerConnected(int(Params[2]),0,activator)

        else:
                whoami.Say('Say enter to request entry')
