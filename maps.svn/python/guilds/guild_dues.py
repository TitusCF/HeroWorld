"""
Script for paying Guild Dues, and to handle Jack in the mainfloor.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# author:Avion temitchell@sourceforge.net
#
# Heavily modified by Nicolas Weeger, 2010-11-14
"""

import Crossfire
import CFGuilds
import CFItemBroker
import random
import CFBank
import CFLog
from CFGuildClearance import CheckClearance

# List of amounts a player can pay
CoinTypes={
    "SILVER":1,
    "GOLD":10,
    "PLATINUM":50,
    "JADE":5000,
    "AMBERIUM":500000,
    "IMPERIAL NOTE":10000,
    "TEN IMPERIAL NOTE":100000,
    "ONE HUNDRED IMPERIAL NOTE":1000000 }

# Archetypes for the 'withdraw' command
ArchType={"SILVER":"silvercoin","GOLD":"goldcoin","PLATINUM":"platinacoin","JADE":"jadecoin","AMBERIUM":"ambercoin", "IMPERIAL NOTE":"imperial","TEN IMPERIAL NOTE":"imperial10","ONE HUNDRED IMPERIAL NOTE":"imperial100"}

# Filename for the bank
bankdatabase = "ImperialBank_DB"

remarklist = ['Excellent','Thank You','Thank You','Thank You', 'Thank You', 'Great', 'OK', 'Wonderful', 'Swell', 'Dude', 'Big Spender']
exclaimlist = ['Hey','Hey','Hey','Hey', 'Now just a minute', 'AHEM', 'OK...Wait a minute', 'Look chowderhead']
buddylist =   ['buddy','buddy','buddy','buddy','pal','friend','friend','friend','friend','dude','chum', 'sweetie']
whoami=Crossfire.WhoAmI()
activator = Crossfire.WhoIsActivator()

def formatted_amount(amount):
    """ Format a price as a string, giving full coin descriptions. Returns 'no money' if amount is 0. """
    if amount == 0:
        return 'no money'
    
    return Crossfire.CostStringFromValue(amount)

def find_mailbox(object):
    while (object.Name != 'mailbox'):
        object = object.Above
        if not object:
                return 0
    return object
def FindCoin(object):
    while (object.Name.find('silver coin')==-1):
        object = object.Above
        if not object:
                return 0
    return object

class GuildDues:
    def __init__(self):
        '''Standard constructor'''
        self.guildname = Crossfire.ScriptParameters()

    def do_help(self):
        '''Handle the 'help' and 'yes' commands.'''
        gm = ''

        if CheckClearance([self.guildname, "GuildMaster"], activator):
            gm = 'You can purchase guild extensions or new desks with the "buy" word.\n'

        whoami.Say('Let me know how much you want to pay. Say pay <amount> <cointype>'
        + '\n\tValid coin types are '
        + ', '.join(CoinTypes)
        + '.\nYou can check the balance by saying "balance".\n'
        + gm
        + 'I also provide mailscrolls at 10 platinum each, use "mailscroll".')

    def do_buy(self, text):
        '''Handles buying a guild extension'''

        if (not CheckClearance([self.guildname, "GuildMaster"], activator)):
                whoami.Say("Only guild masters and GMs can buy extensions or new desks for the guild.")
                return

        # This lists all items that can be bought, extensions or new desks
        Items = {
            'Stove':5000*12,
            'Cauldron':5000*12,
            "TanningDesk":5000*12,
            "ThaumaturgyDesk":5000*12,
            "JewelersBench":5000*24,
            "BowyerBench":5000*10,
            "Forge":5000*10,

            'BBQ':500000*12,
            'AlchemyLab':500000*24,
            "CrystalRoom":500000*12,
            "Tannery":500000*12,
            "JewelersRoom":500000*24,
            "ThaumaturgyRoom":500000*12,
            "Bowyer":500000*12,
            "Smithy":500000*12}

        # This is the list of cards to buy new desks
        Cards = ["Stove", "Cauldron", "TanningDesk", "ThaumaturgyDesk", "JewelersBench", "BowyerBench", "Forge"]

        mymap = whoami.Map
        path = mymap.Path
        path = path.replace("mainfloor", "")
        SecondFloor = Crossfire.ReadyMap(path + 'secondfloor')
        ToolShed = Crossfire.ReadyMap(path + "guild_toolshed")

        # This is the list of guild extensions, and the coordinates of the activation button
        Rooms = {
            "BBQ": (mymap, 40, 25),
            "AlchemyLab": (SecondFloor, 22, 12),
            "CrystalRoom": (SecondFloor, 22, 13),
            "Tannery": (SecondFloor, 22, 14),
            "ThaumaturgyRoom": (SecondFloor, 21, 13),
            "JewelersRoom": (SecondFloor, 21, 14),
            "Bowyer": (ToolShed, 22, 16),
            "Smithy": (ToolShed, 23, 16) }

        if len(text) == 1:
            help = "Buy what?\nYou can buy:\n"

            for i in Items:
                if i in Cards:
                    help += " - " + i + " (card): " + formatted_amount(Items[i]) + "\n"
                else:
                    Loc = Rooms.get(i)
                    coin = Loc[0].ObjectAt(Loc[1], Loc[2])
                    coin = FindCoin(coin)

                    if coin != 0:
                        continue

                    help += " - " + i + " (extension): " + formatted_amount(Items[i]) + "\n"

            whoami.Say(help)
            return

        item = text[1]
        if not item in Items.keys():
            whoami.Say("I don't know that item, sorry")
            return

        Price = Items.get(item)
        bank = CFBank.CFBank(bankdatabase)
        balance = bank.getbalance(self.accountname)
        if Price > balance:
            whoami.Say("The guild does not have sufficient funds.")
            return

        if item in Cards:
                card = activator.CreateObject('diploma')
                card.Name = item
                card.Message = 'This enables you to buy a new ' + item + ' for your guild.'
                card.Value = 0
                bank.withdraw(self.accountname, Price)
                whoami.Say("Here is your card\nThe guild now has %s on account." %(formatted_amount(bank.getbalance(self.accountname))))

                return

        Loc = Rooms.get(item)
        coin = Loc[0].ObjectAt(Loc[1], Loc[2])
        coin = FindCoin(coin)

        if coin != 0:
            whoami.Say("The guild already has this expansion!")
            return

        coin = mymap.CreateObject('silvercoin',40,29)
        coin.Teleport(Loc[0] ,Loc[1], Loc[2])

        bank.withdraw(self.accountname, Price)
        whoami.Say("The new room has been unlocked.\n"
        + "The guild now has %s on account." %(formatted_amount(bank.getbalance(self.accountname))))

    def do_mailscroll(self, text):
        '''Handle getting a mailscroll for a friend.'''
        if len(text) == 1:
            whoami.Say('Usage "mailscroll <friend>"')
            return

        log = CFLog.CFLog()

        if not log.info(text[1]):
            whoami.Say('I don\'t know %s'%text[1])
            return

        priceMailScroll = 5
        priceFactor = 50	# platinum to silver conversion

        if activator.PayAmount(priceMailScroll*priceFactor):
                whoami.Say('Here is your mailscroll to %s'%text[1])
                id = activator.CreateObject('scroll')
                id.Name = 'mailscroll T: '+text[1]+' F: '+activator.Name
                id.NamePl = 'mailscrolls T: '+text[1]+' F: '+activator.Name
                id.Value = 0
        else:
                whoami.Say('You need %s platinum for a mailscroll'%priceMailScroll)

    def do_balance(self):
        '''Handle the display of the guild's balance.'''
        bank = CFBank.CFBank(bankdatabase)
        balance = bank.getbalance(self.accountname)
        whoami.Say("The guild currently has %s on account." %(formatted_amount(balance)))

    def do_pay(self, text):
        '''Handle player paying dues to the guild.'''
        if len(text) < 2:
            whoami.Say("How much ya wanna pay %s?\nYou can specify amounts in "%(random.choice(buddylist)) +
            ', '.join(i.lower() for i in CoinTypes.keys()))
            return

        cost = text[1]
        type = ' '.join(text[2:])
        utype = type.upper()
        if not utype in CoinTypes.keys():
            whoami.Say("Sorry, I don't know what are %s"%type)
            return

        conversionfactor = CoinTypes.get(utype)
        total = int(cost)*conversionfactor
        if activator.PayAmount(total):
            guild = CFGuilds.CFGuild(self.guildname)
            guild.pay_dues(activator.Name,total)
            whoami.Say("%s, %s %s paid to the guild." %(random.choice(remarklist), cost, type))
            bank = CFBank.CFBank(bankdatabase)
            bank.deposit(self.accountname, total)
        else:
            if cost > 1:
                whoami.Say("%s, you don't have %s %ss." %(random.choice(exclaimlist), cost, type))
            else:
                whoami.Say("You don't have any %s, %s." %(type, random.choice(buddylist)))

    def do_withdraw(self, text):
        if (not activator.DungeonMaster==1 and not CheckClearance([self.guildname,"Master"],activator)):
            whoami.Say("Only guild masters, masters, and DMs can withdraw funds from the guild.")
            return

        try:
            Amount=int(text[1])
        except:
            whoami.Say("Usage: withdraw <quantity> {cointype=silver}")
            return

        bank = CFBank.CFBank(bankdatabase)
        balance = bank.getbalance(self.accountname)

        if len(text) > 2:
            Type = ' '.join(text[2:])
        else:
            Type = "silver"

        if not Type.upper() in CoinTypes.keys():
            whoami.Say("Sorry, I have no clue what %s are"%Type)
            return

        Value = CoinTypes.get(Type.upper())

        if Amount*Value <= balance:
                message = (str(Amount))
                message +=" " + Type + " withdrawn.\nYour new present balance is "

                id = activator.CreateObject(ArchType.get(Type.upper()))
                CFItemBroker.Item(id).add(Amount)
                bank.withdraw(self.accountname, Amount*Value)
                message += formatted_amount(bank.getbalance(self.accountname))+"."
                whoami.Say(message)
        else:
                message="You only have " + formatted_amount(bank.getbalance(self.accountname))+" on your account."
                whoami.Say(message)

    def handle_jack(self):
        '''Handle Jack, the guild helper'''

        text = Crossfire.WhatIsMessage().split()
        command = text[0].lower()

        if command == 'buy':
            self.do_buy(text)
            return

        if command == 'mailscroll':
            self.do_mailscroll(text)
            return

        if command == 'balance':
            self.do_balance()
            return

        if command == 'pay':
            self.do_pay(text)
            return

        if command == 'withdraw':
            self.do_withdraw(text)
            return

        if command == 'help' or command == 'yes':
            self.do_help()
            return

        message = "Howdy %s, paying some guild dues today?" %(random.choice(buddylist))
        whoami.Say(message)


    def handle(self):
        '''Main handling function'''
        if not self.guildname:
            activator.Write('dues error, please notify a DM')
            return

        bank = CFBank.CFBank(bankdatabase)
        self.accountname = self.guildname + str(self.guildname.__hash__())

        if whoami.Name == 'Jack':
            self.handle_jack()
            return

        amount = Crossfire.WhoIsOther().Value * Crossfire.WhoIsOther().Quantity
        bank.deposit(self.accountname, amount)

dues = GuildDues()
dues.handle()
