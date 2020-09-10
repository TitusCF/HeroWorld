import Crossfire

me = Crossfire.WhoAmI()
ac = Crossfire.WhoIsActivator()

# It is assumed that this buffed weapon uses PermExp to denote how much it has been used.
if me.PermExp > 0:
    me.Str = me.Archetype.Clone.Str
    me.Dex = me.Archetype.Clone.Dex
    me.Con = me.Archetype.Clone.Con
    me.Int = me.Archetype.Clone.Int
    me.Pow = me.Archetype.Clone.Pow
    me.Wis = me.Archetype.Clone.Wis
    me.Cha = me.Archetype.Clone.Cha
    me.HP = me.Archetype.Clone.HP
    me.SP = me.Archetype.Clone.SP
    me.Grace = me.Archetype.Clone.Grace
    me.LastSP = me.Archetype.Clone.LastSP
    me.WC = me.Archetype.Clone.WC
    me.AC = me.Archetype.Clone.AC
    me.Dam = me.Archetype.Clone.Dam
    me.Weight = me.Archetype.Clone.Weight
    me.AttackType = me.Archetype.Clone.AttackType
    me.Food = me.Archetype.Clone.Food
    # Experience should be affected before Item Power, since it affects that field
    me.AddExp(-me.TotalExp)
    me.ItemPower = me.Archetype.Clone.ItemPower
    ac.Write("The "+me.Name+" shudders and looks almost like a normal weapon again.")
