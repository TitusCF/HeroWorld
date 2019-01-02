import Crossfire
import random

def tame_angry_pets():
    """
    Look for pets (monsters owned by the player) who are not marked as friendly.
    Then try to re-pet them with a 1 in 10 change to fail.    
    
    Some pets miss their Friendly flag only, others also miss the IsPet property.
    Others also have the wrong value for attack_movement which should be PETMOVE for pets.
    """
    
    PETMOVE = Crossfire.AttackMovement.PETMOVE
    player = Crossfire.WhoIsActivator()
    if player.Type != Crossfire.Type.PLAYER:
        return
    Crossfire.SetReturnValue( 1 )
    
    fizzle = True 
    
    #Can't do this check by looking at the FriendlyList because we are looking for
    #those pets who are *not* in that list. So check for the monsters in the same map
    #as the player, owned by it, and yet not friendly.
    #Is there a better way than checking all _items_ in the map?
    for w in range(player.X-5, player.X+5):
        if w<1 or w>range(player.Map.Width):
            continue
        for h in range(player.Y-5, player.Y+5):
            if h<1 or h>range(player.Map.Height):
                continue
            obj = player.Map.ObjectAt(w,h) 
            while obj != None:
                if obj.Monster and obj.Owner == player and not obj.Friendly: #angry pet
                    fizzle = False
                    if random.randint(0,9): #tame
                        obj.Friendly = True
                        if not obj.IsPet:
                            obj.IsPet = True
                        if obj.AttackMovement != PETMOVE:
                            obj.AttackMovement = PETMOVE
                        player.Write( 'Your %s looks at you tenderly, fearless at your enemies.' % obj.Name )
                    else: #fail
                        player.Write( 'Your %s is still angry with you.'  % obj.Name )
                obj = obj.Above
    if fizzle:
        player.Write('Fzzzzzzzz...')

tame_angry_pets()
