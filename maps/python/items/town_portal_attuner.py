import Crossfire

def main():
    attuner = Crossfire.WhoAmI()
    player = Crossfire.WhoIsActivator()
    player.WriteKey("town_portal_dest_map", attuner.Map.Path, 1)
    player.WriteKey("town_portal_dest_x", str(attuner.X), 1)
    player.WriteKey("town_portal_dest_y", str(attuner.Y), 1)
    player.Write("You attune your town portal here.")
    return 1

Crossfire.SetReturnValue(main())
