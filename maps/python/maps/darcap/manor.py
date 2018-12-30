import random
# -*- coding: utf-8 -*-
# Script handling various things for Darcap Manor.
#
# It is used:
# - in the blue zone, to count how many kobolds are on the map
# - for the potion, to explode walls in the treasure room
# - in the brown zone, to prevent Kaptel's death or trigger the opening of the exit
# - in the white zone, to check if the player knows how many of a certain jewel there are

import Crossfire

def blue_count_kobolds():
    map = Crossfire.WhoAmI().Map
    count = 0
    for x in range(map.Width):
        for y in range(map.Height):
            below = map.ObjectAt(x, y)
            while below:
                if below.Name == 'kobold':
                    count = count + 1
                    break
                below = below.Above

    return count

def blue_check():
    '''Player triggered the lever, check if correct number of kobolds. '''

    if blue_count_kobolds() != 4:
        Crossfire.SetReturnValue(1)
        Crossfire.WhoIsActivator().Message('The level will only activate if there are exactly 4 kobolds on the map')
        return

def potion_find_wall(map, x, y, name):
    '''Find the wall of a given archetype at the specified location.'''
    item = map.ObjectAt(x, y)
    while item:
        if item.ArchName == name:
            return item
        item = item.Above

    return None

def potion_check():
    """Handle the Darcap Manor's potion being thrown. Check if walls to destroy in the treasure room."""

    # note: duplication with /CFMove.py, should be factorised at some point
    dir_x = [  0, 0, 1, 1, 1, 0, -1, -1, -1 ]
    dir_y = [ 0, -1, -1, 0, 1, 1, 1, 0, -1 ]

    env = Crossfire.WhoAmI().Env

    if env.Map.Path != '/darcap/darcap/manor.treasure':
        return

    x = env.X + dir_x[ env.Direction ]
    y = env.Y + dir_y[ env.Direction ]

    if y != 4 and y != 8 and y != 12:
        return

    if x != 9 and x != 10:
        return

    left = potion_find_wall(env.Map, 9, y, 'cwall_mural_1_1')
    right = potion_find_wall(env.Map, 10, y, 'cwall_mural_1_2')

    if left == None or right == None:
        # hu?
        return

    left.Remove()
    right.Remove()

    env.Map.CreateObject('rubble', 9, y)
    env.Map.CreateObject('rubble', 10, y)

    Crossfire.WhoAmI().Remove()

    env.Map.Print('The wall explodes!')

def kaptel_death():
    '''
    Handle Kaptel's death event. Depending on whether the player triggered the lever,
    either prevent death or trigger the opening of the exit.
    '''
    who = Crossfire.WhoAmI()

    floor = who.Map.ObjectAt(24, 1)
    while floor != None and floor.Above != None:
        floor = floor.Above

    if floor.Name == 'boulder':
        who.Say('AAAAAAAAAaaaaahhhhhhhhhhhhhh!!!!!!!!!')
        who.Map.TriggerConnected(11, 1)
        return

    who.Map.Print("%s roars and seems to regenerate!"%(Crossfire.WhoAmI().Name))
    who.HP = who.MaxHP / 2
    Crossfire.SetReturnValue(1)

def challenge_correct_reply(count, msg):
    if count == 0:
        return msg == '0' or msg == 'none'
    return count == int(msg)

def check_arch(map, x, y, arch):
    below = map.ObjectAt(x, y)
    while below:
        if below.ArchName == arch:
            return 1
        below = below.Above
    
    return 0

def white_challenge():
    '''
    Say handler for the final white challenge.
    Player must know how many jewel piles of a certain kind were on the map, else failure.
    '''

    msg = Crossfire.WhatIsMessage()
    ear = Crossfire.WhoAmI()
    status = ear.ReadKey('challenge')

    if status == 'done':
        ear.Say('You already replied!')
        return

    if status == '':
        if msg == 'ready':
            ear.WriteKey('challenge', 'ready', 1)
            ear.Say('To get the white key, reply to the following question:')

            archs = [ 'gem', 'amethyst', 'emerald', 'ruby', 'sapphire', 'pearl' ]
            names = [ 'diamonds', 'amethysts', 'emeralds', 'rubies', 'sapphires', 'pearls' ]
            choice = random.randint(0, 5)

            location = random.randint(1, 3)
            if location == 1:
                positions = [ [8, 2], [10, 7], [10, 9], [28, 18] ]
                where = 'behind windows'
            elif location == 2:
                positions = [ [10, 7], [10, 9], [2, 23], [16, 21], [8, 2], [24, 6], [23, 7], [25, 7], [19, 26], [20, 26], [19, 28] ]
                where = 'behind grates'
            else:
                positions = [ [8, 2], [10, 7], [10, 9], [2, 23], [16, 21], [8, 2], [24, 6], [23, 7], [25, 7], [28, 18], [19, 26], [20, 26], [19, 28] ]
                where = 'in this dungeon'

            count = 0
            for pos in positions:
                count += check_arch(ear.Map, pos[0], pos[1], archs[choice])

            ear.Say('How many piles of %s did you see %s?'%(names[choice],where))
            ear.Say('Reply with a number in digits please.')

            # keep reply in  the ear, easier
            ear.WriteKey('reply', str(count), 1)
        return

    # check answer

    ear.WriteKey('challenge', 'done')

    count = int(ear.ReadKey('reply'))
    if challenge_correct_reply(count, msg):
        ear.Map.TriggerConnected(60, 1)
        ear.Say('Correct!')
    else:
        ear.Say('Sorry, that is not the correct reply. Please try again later.')

if Crossfire.ScriptParameters() == 'blue':
    blue_check()
elif Crossfire.ScriptParameters() == 'potion':
    potion_check()
elif Crossfire.ScriptParameters() == 'kaptel':
    kaptel_death()
elif Crossfire.ScriptParameters() == 'white':
    white_challenge()
