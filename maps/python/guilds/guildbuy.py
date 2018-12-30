import Crossfire
import CFGuilds

whoami=Crossfire.WhoAmI()
guildname=Crossfire.ScriptParameters() # 1 is 'apply' event

def find_player(object):
    while (object.Type != 1) : #1 is type 'Player'
        object = object.Above
        if not object:
            return 0
    return object

activator=Crossfire.WhoIsActivator()
map = activator.Map

players = []
names = []

if (guildname):
    #find players by coords
    ob1=map.ObjectAt(33,24)
    ob2=map.ObjectAt(33,26)
    objects = [ob1, ob2]
    for object in objects:
        temp = find_player(object)
        if temp:
            players.append(temp)
    players.append(activator)

    for player in players:
        names.append(player.Name)

    if len(players) == 3:
        print '%s,%s and %s found guild %s' %(names[0], names[1], names[2], guildname)

        CFGuilds.CFGuildHouses().establish(guildname)
        #Masterize them
        for player, name in zip(players, names):
            CFGuilds.CFGuild(guildname).add_member(name, 'GuildMaster')

            #teleport them
            player.Teleport(map,int(11),int(16))
            message = "You have purchased the %s guild.  Rule it wisely.  (I would type 'save' right about now...)"%guildname
    elif len(players)==2:
            message="To purchase a guild requires one additional person to stand on the alcoves above."
    else:
        message = 'To purchase a guild requires two additional persons to stand on the alcoves above.'
else:
    print 'Guild Purchase Error: %s, %s' %(guildname, activatorname)
    message = 'Guild Purchase Error, please notify a DM'

whoami.Say(message)
