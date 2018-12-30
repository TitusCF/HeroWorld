# -*- coding: utf-8 -*-
#
# This script makes players aware of the time, tracking the current time
import Crossfire

def ring_scorn(pl):
    god = pl.God
    if (god == 'Devourers' or god == 'Sorig' or god == 'Ruggilli' or god == 'Gaea' or god == 'Mostrai' or god == 'Lythander'):
        pl.Message('You hear the bell of the glorious temple of %s'%god)
    elif (god == 'Valriel' or god == 'Gorokh'):
        pl.Message('You hear the bell of the glorious church of %s'%god)
    else:
        pl.Message("You hear the bells of the various temples of Scorn.")

def ring_darcap(pl):
    god = pl.God
    if (god == 'Devoureres'):
        pl.Message('You hear the glorious bell of St Andreas')
    else:
        pl.Message("You hear the bell of St Andreas.")

def ring_navar(pl):
    god = pl.God
    if (god == 'Gorokh' or god == 'Ruggilli' or god == 'Sorig' or god == 'Valriel'):
        pl.Message('You hear the bell of the glorious temple of %s'%god)
    elif (god == 'Mostrai'):
        pl.Message('You hear the bell of Mostrai\'s glorious cathedral')
    elif (god == 'Gaea'):
        pl.Message('You hear the bell of Gaea\'s glorious shrine')
    else:
        pl.Message("You hear the bells of the temples of Navar.")

def ring_bell():
    players = Crossfire.GetPlayers()
    for player in players:
        if player.Map == None:
            continue
        if player.Map.Region == None:
            continue
        if player.Map.Region.Name == 'scorn':
            ring_scorn(player)
        elif player.Map.Region.Name == 'darcap':
            ring_darcap(player)
        elif player.Map.Region.Name == 'navar':
            ring_navar(player)

dict = Crossfire.GetPrivateDictionary()
hour = Crossfire.GetTime()[3]

if not 'init' in dict.keys():
    dict['init'] = 1
    dict['last'] = hour
    Crossfire.Log(Crossfire.LogDebug, "Bell init")
else:
    last = dict['last']
    if (hour != last):
        dict['last'] = hour
        Crossfire.Log(Crossfire.LogDebug, "Bell ringing")
        ring_bell()
