# -*- coding: utf-8 -*-
# QuestConditionalDrop.py - A script to let items be dropped only if a quest
# reached a certain step.
#
# This script should be called for the 'death' event of a living thing.
# Each item in inventory will be checked for a 'drop_if_quest' key, which format is:
# - quest name
# - a list of steps, either single value (x) or range (x-y)
# If any matches, then the item will be dropped, else it won't.
# Items without the 'drop_if_quest' key are not affected.

import Crossfire

whoami = Crossfire.WhoAmI()
killer = Crossfire.WhoIsActivator()

def matches(rule):
    if rule == '':
        return True
    args = rule.split()

    currentstep = killer.QuestGetState(args[0])
    for rule in args[1:]:
        if rule.find("-") == -1:
            startstep = int(rule)
            endstep = startstep
        else:
            startstep = int(rule.split("-")[0])
            endstep= int(rule.split("-")[1])
        if currentstep >= startstep and currentstep <= endstep:
            return True

    return False


inv = whoami.Inventory
while inv != None:
    key = inv.ReadKey('drop_if_quest')
    if not matches(key):
        inv.GodGiven = True
    inv = inv.Below

