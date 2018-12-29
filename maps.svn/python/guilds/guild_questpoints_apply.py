import Crossfire
import CFGuilds

activator=Crossfire.WhoIsActivator()
activatorname=activator.Name
whoami=Crossfire.WhoAmI()
mymap = activator.Map
mapname = mymap.Name
trank = 0

points=Crossfire.ScriptParameters() # 1 is apply event

if points:
    guild = CFGuilds.SearchGuilds(activatorname)
    if guild:
        CFGuilds.CFGuild.add_questpoints(activatorname,points)
    else:
        pass
else:
    print 'Error, no points specified in %s on map %s' %(whoami,mapname)
