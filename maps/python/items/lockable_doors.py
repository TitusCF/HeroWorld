# Script for the lockable door and key.
# Ideas courtesy Yann Chachkoff.
#
# Copyright 2012 Nicolas Weeger
# Released as GPL
#
# Lockable doors have 2 components:
# - doors, which have the 'lockable 1' attribute
# - locking keys, which have a apply handler pointing to this script
#
# Rules are as follow:
# - the 'slaying' field is used to match doors and keys
# - locking keys start blank (empty slaying)
# - applying a blank key to an unlocked door locks the door and assigns the door to the key,
#  the key gets a '(used)' appended to its name
# - applying a valid key to a locked/unlocked door unlocks/locks
# - locked doors must be alive, unlocked mustn't be
# - the 'other_arch' field is used to link locked and unlocked variants
# - the following fields are changed when un/locking takes place: name, move_block,
#  move_allow, face. This allows customization of the doors (prevent walk but not fly, for instance)
#
# Right now, the following archetypes use this system:
# lockable_vdoor, lockable_vdoor_locked, lockable_hdoor, lockable_hdoor_locked, locking_key
#
# Feel free to use those as a base.
#
# Note: using a DOOR (type 23) instead of a LOCKED_DOOR (type 20) doesn't seem to
# work, since any key can remove the door - not the desired behaviour.
#
# If the lockable door has the correct 'trigger' hook (case for the previous archetypes),
# then the door can be picked, depending on the player's lockpicking level.
# Experience is awarded for the first successful pick only. Failures decrease the
# chance to succeed, and increase the chance of leaving traces.

import Crossfire
import random

def get_door(me, direction):
    '''Find the first item in the specified direction'''
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

def give_properties(who, lock):
  '''Give properties from either the archetype or the other archetype.'''
  if lock == who.Archetype.Clone.Alive:
    arch = who.Archetype
  else:
    arch = who.OtherArchetype
  who.MoveType = arch.Clone.MoveType
  who.MoveBlock = arch.Clone.MoveBlock
  who.MoveAllow = arch.Clone.MoveAllow
  who.Alive = arch.Clone.Alive
  who.Name = arch.Clone.Name
  who.Face = arch.Clone.Face

def check_picked(door, who):
  '''Check if the lock was picked, and inform the player if this is the case.'''
  if door.ReadKey('door_picked') == '1':
    who.Write('You notice some suspicious traces on the lock.')
    door.WriteKey('door_picked', '')

def handle_key():
  '''Handle applying a locking key.'''
  Crossfire.SetReturnValue(1)

  key = Crossfire.WhoAmI()
  who = Crossfire.WhoIsActivator()

  door = get_door(who, who.Facing)

  while door != None:
    if door.ReadKey('lockable') == '1':
      break
    door = door.Above

  if door == None:
    who.Write('There is no lock here')
  else:
    if door.Alive:
      # door is locked, check if key matches
      if door.Slaying != key.Slaying:
        who.Write("You can't use this %s on this %s"%(key.Name, door.Name))
      else:
        who.Write('You unlock the %s'%(door.Name))
        give_properties(door, 0)
        check_picked(door, who)
    else:
      # door is unlocked, a key can lock if blank or matching
      if key.Slaying == '' or key.Slaying == None or key.Slaying == door.Slaying:
        if key.Slaying == '' or key.Slaying == None:
          key.Slaying = door.Slaying
          key.Name = key.Name + " (used)"
        who.Write('You lock the %s'%(door.Name))
        give_properties(door, 1)
        check_picked(door, who)
      else:
        who.Write("You can't use this %s on this %s"%(key.Name, door.Name))

def get_attempts(door, who):
  '''Get how many attempts to pick a lock a player did'''
  s = door.ReadKey('attempts_' + who.Name)
  if s == '':
    return 0
  return int(s)

def get_success_chance(door, who, level):
  '''Return the chance of successfully picking the lock for the player'''
  if door.ReadKey('was_picked_' + who.Name) != '':
    return 100
  attempts = min(get_attempts(door, who), 5)
  diff = level - door.Level - attempts
  #who.Write('chance: %d'%(max(25, min(90, 50 + diff * 5))))
  return max(25, min(90, 50 + diff * 5))

def get_exp(door, who):
  '''Return the experience for lockpicking the door'''
  if door.ReadKey('was_picked_' + who.Name) != '':
    return 0
  attempts = get_attempts(door, who)
  #who.Write('exp: %d'%(round((door.Exp * (100. - min(100., attempts * 20.))) / 100.)))
  return round((door.Exp * (100. - min(100., attempts * 20.))) / 100.)

def handle_lockpick():
  '''Handle lockpicking a door.'''
  Crossfire.SetReturnValue(1)

  door = Crossfire.WhoAmI()
  who = Crossfire.WhoIsActivator()

  if who == None:
    return

  if door.Alive == 0:
    who.Write("This %s is unlocked."%(door.Name))
    return

  chance = get_success_chance(door, who, Crossfire.WhoIsOther().Level)
  # chance to leave traces on the lock
  if random.randint(0, 100) < 100 - chance:
    door.WriteKey('door_picked', '1', 1)

  # attempt to unlock
  if random.randint(0, 100) < chance:
    who.Write('You successfully pick the lock.')
    give_properties(door, 0)
    who.AddExp(get_exp(door, who), Crossfire.WhoIsOther().Name)
    door.WriteKey('was_picked_' + who.Name, '1', 1)
  else:
    who.Write('You fail to pick the lock.')
    door.WriteKey('attempts_' + who.Name, str(get_attempts(door, who) + 1), 1)

event = Crossfire.WhatIsEvent()
if event.Subtype == Crossfire.EventType.APPLY:
  handle_key()
elif event.Subtype == Crossfire.EventType.TRIGGER:
  handle_lockpick()
