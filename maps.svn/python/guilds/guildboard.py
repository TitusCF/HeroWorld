import Crossfire
import CFGuilds

def mycmp(a, b):
    return cmp(a[1], b[1])

activator=Crossfire.WhoIsActivator()
guilds = CFGuilds.CFGuildHouses()

activator.Write('Guild Standings:')
activator.Write('Guild - Points - Status')

guildlist = guilds.list_guilds()
standings = []
for guild in guildlist:
    record = guilds.info(guild)
    if record['Status'] != 'inactive':
        standings.append([record['Points'], guild, record['Status']])
standings.sort(mycmp)
for item in standings:
    activator.Write('%s - %s - %s' %(item[1],item[0],item[2]))

Crossfire.SetReturnValue(1)
