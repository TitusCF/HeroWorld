#
# This module will automaticall arrest players killing other players,
# provided the option was activated by a DM through 'autojail 1'.
# Note the settings defaults to 0 and isn't kept during server resets.

import Crossfire


def check_autojail():
    killer = Crossfire.WhoIsActivator()

    if killer.Type != Crossfire.Type.PLAYER or killer.DungeonMaster:
        return

    dict = Crossfire.GetSharedDictionary()
    if not 'autojail' in dict or dict['autojail'] != 1:
        return

    victim = Crossfire.WhoAmI()
    killer.Message('You are auto-jailed for PKing %s'%victim.Name)
    ret = killer.Arrest()
    if ret == 0:
        msg = '%s was auto-jailed for PKing %s'%(killer.Name, victim.Name)
    else:
        msg = 'Failed to auto-jail %s for PKing %s, code %d'%(killer.Name, victim.Name, ret)
    players = Crossfire.GetPlayers()
    for player in players:
        if player.DungeonMaster:
            player.Message(msg)

check_autojail()
