import Crossfire

killer = Crossfire.WhoIsActivator()
# If the killing piece has no owner (so we are in hand-to-hand combat)
# and the killer is a player (who would be wielding this weapon.
if killer.Owner is None and killer.Type == Crossfire.Type.PLAYER:
    # Find the equipped weapon on the player.
    inv = killer.Inventory
    weap = None
    while inv:
        # The artifact name "of Souls" does this.
        # Also only do things if the weapon is cursed as a sanity check.
        if inv.Applied == 1 and inv.Cursed == 1 and inv.Title == 'of Souls':
            weap = inv
            break
        inv = inv.Below
    # Don't bother with xp if weapon is missing or already at the maximum level.
    if weap != None and weap.ItemPower < 115:
        # Get the victim -- we need to know how much exp they are worth.
        victim = Crossfire.WhoAmI()
        if victim is not None:
            old_level = weap.ItemPower
            old_xp = weap.TotalExp
            # Add experience to the weapon (though we only care about the total_exp field)
            # As the weapon gets stronger, it takes a larger share of the exp
            weap.AddExp((victim.Exp * (1.0 + weap.ItemPower / 115.0)) // 1)
            
            # Determine the change in XP
            delta_exp = weap.TotalExp - old_xp
            
            # DEBUGGING INFO:
            # Crossfire.Log(Crossfire.LogInfo, str(victim.Exp) + " -> " + str(delta_exp) + " & " + str(victim.Exp - delta_exp))
            
            # Reduce the XP of the kill -- the sword has taken the xp rather than you
            victim.Exp -= delta_exp
            
            # Note: For weapons, AddExp sets ItemPower to hold the level instead of level.
            # Always check for item power exceeding your limit
            exceed_item_power = ((killer.ItemPower + weap.ItemPower - old_level) > killer.Level)
            
            # DEBUGGING INFO:
            #Crossfire.Log(Crossfire.LogInfo, str(killer.ItemPower) + ": " + str(weap.ItemPower) + " (" + str(old_level) + ")")
            
            # Adjust the equipping creature's item power to match the new item power.
            if weap.ItemPower != old_level:
                killer.ItemPower = killer.ItemPower + weap.ItemPower - old_level
            
            # If we aren't levelling up, make sure we don't have an item power overload anyway.
            # This makes player levelups from noncombat undo the debuff on the next kill.
            # Use a negative Str to denote the debuff -- any attribute that is always positive would do, though.
            do_level_up = old_level != weap.ItemPower or exceed_item_power or weap.Str < 0
            
            # If the experience change yields a level-up or we need to check for removal of debuff, then buff the sword.
            if do_level_up == True:
                # Display a message about the sword growing stronger
                killer.Write("You can feel the "+weap.Name+" pulse darkly in your hand.")
                # Apply the buffs to the sword
                # If we exceed the player's item_power threshold, then give player a massive debuff instead.
                if exceed_item_power == True:
                    # Only print the message when we flip from buff to debuff.
                    if weap.Str >= 0:
                        killer.Write("The "+weap.Name+" is overwhelming your body!")
                    weap.Str = -15
                    weap.Dex = -15
                    weap.Con = -15
                    weap.Int = -15
                    weap.Pow = -15
                    weap.Wis = -15
                    weap.Cha = -15
                    weap.HP = -15
                    weap.SP = -15
                    weap.Grace = -15
                    weap.LastSP = weap.Archetype.Clone.LastSP * 2
                    weap.WC = -15
                    weap.AC = -15
                    weap.Dam = -15
                    weap.Food = -15
                else:
                    weap.Str = weap.Archetype.Clone.Str + weap.ItemPower // 10
                    weap.Dam = weap.Archetype.Clone.Dam + weap.ItemPower // 2
                    weap.WC = weap.Archetype.Clone.WC + weap.ItemPower // 5
                    weap.HP = weap.Archetype.Clone.HP - 10 + weap.ItemPower
                    weap.SP = weap.Archetype.Clone.SP - 10 + weap.ItemPower // 17
                    weap.Grace = weap.Archetype.Clone.Grace - 10 + weap.ItemPower // 19
                    weap.Food = weap.Archetype.Clone.Food - 10 + weap.ItemPower // 23
                    weap.LastSP = weap.Archetype.Clone.LastSP - weap.ItemPower // 11
                    if weap.LastSP < 0:
                        weap.LastSP = 0;
                # Weapon Weight is not affected by whether it overloads you or not.
                weap.Weight = weap.Archetype.Clone.Weight + 100 * weap.ItemPower
                # Give the weapon additional attacktypes as it grows stronger
                if weap.ItemPower >= 115:
                    weap.AttackType = weap.Archetype.Clone.AttackType + Crossfire.AttackType.WEAPONMAGIC + Crossfire.AttackType.GODPOWER + Crossfire.AttackType.PARALYZE + Crossfire.AttackType.DEPLETE + Crossfire.AttackType.LIFE_STEALING
                elif weap.ItemPower >= 52:
                    weap.AttackType = weap.Archetype.Clone.AttackType + Crossfire.AttackType.WEAPONMAGIC + Crossfire.AttackType.PARALYZE + Crossfire.AttackType.DEPLETE + Crossfire.AttackType.LIFE_STEALING
                elif weap.ItemPower >= 39:
                    weap.AttackType = weap.Archetype.Clone.AttackType + Crossfire.AttackType.PARALYZE + Crossfire.AttackType.DEPLETE + Crossfire.AttackType.LIFE_STEALING
                elif weap.ItemPower >= 26:
                    weap.AttackType = weap.Archetype.Clone.AttackType + Crossfire.AttackType.DEPLETE + Crossfire.AttackType.LIFE_STEALING
                elif weap.ItemPower >= 13:
                    weap.AttackType = weap.Archetype.Clone.AttackType + Crossfire.AttackType.LIFE_STEALING
                else:
                    weap.AttackType = weap.Archetype.Clone.AttackType
                # The buffs apply to the player of their own accord, so don't do anything here.
    # Debugging info
    # else:
    #    Crossfire.Log(Crossfire.LogInfo, "Nope")
