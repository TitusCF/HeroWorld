import CFReputation
import Crossfire

def show_reputation(who):
    reputations = CFReputation.reputation(who.Name)
    lines = []
    if len(reputations) == 0:
        lines.append("You don't have a reputation with anyone.")
    else:
        lines.append("You are known by the following factions:")
        for p in reputations:
            lines.append("\t%s....................%d" % (p[0], p[1]))
    who.Message("\n".join(lines))

show_reputation(Crossfire.WhoAmI())
