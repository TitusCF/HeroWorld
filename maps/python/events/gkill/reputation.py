import Crossfire
import CFReputation

def get_killer(hitter):
    owner = hitter.Owner
    if owner is not None:
        return owner
    return hitter

def is_player(op):
    return op.Type == Crossfire.Type.PLAYER

killer = get_killer(Crossfire.WhoIsActivator())
victim = Crossfire.WhoAmI()

if is_player(killer):
    CFReputation.record_kill(victim.Race, victim.Map.Region.Name, killer.Name)
