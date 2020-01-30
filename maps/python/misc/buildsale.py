# Script for say event of Imperial Bank Tellers (Modified for selling build scrolls)
#
# Copyright (C) 2002 Joris Bontje
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
# The author can be reached via e-mail at jbontje@suespammers.org
#
# Updated to use new path functions in CFPython and broken and
# modified a bit by -Todd Mitchell


import Crossfire

import string
import CFBank
import CFItemBroker

activator = Crossfire.WhoIsActivator()
activatorname = activator.Name
whoami = Crossfire.WhoAmI()
x = activator.X
y = activator.Y

region = '%s' % (activator.Map.Region.Name)

#EASILY SETTABLE PARAMETERS

if ((region == 'navar')
or (region == 'navarempire')
or (region == 'celvear')
or (region == 'lostwages')):
    bankdatabase = "navarianroyalBank_DB"
    costmply = 2
    curname0 = 'Navarian Royal'
    curname1 = 'navarian royal'
    gexcrate = 500
    welcome = 'Welcome to Navarian Architectural Services, A branch of the Bank of Navar. Do you need help?'
else:
    bankdatabase = "ImperialBank_DB"
    costmply = 1
    curname0 = 'Imperial'
    curname1 = 'imperial note'
    gexcrate = 1000
    welcome = 'Welcome to Imperial Architectural Services, A branch of the Bank of Skud. Do you need help?'

tmplcostfnt = 2 * costmply
shopcostfnt = 2 * costmply
wellcostfnt = 5 * costmply
fountaincostfnt = 5 * costmply
farmcostfnt = 1 * costmply
minecostfnt = 3 * costmply
guildcostfnt = 8 * costmply
lampcostfnt = 5 * costmply
houseamount = 1000000 * costmply
templeamount = 2000000 * costmply
shopamount = 2000000 * costmply
tobamount = 10000000 * costmply
wellamount = 500000 * costmply
fountainamount = 500000 * costmply
farmamount = 100000 * costmply
prospectamount = 10 * costmply
mineamount = 3000000 * costmply
harboramount = 1000000 * costmply
guildamount = 8000000 * costmply
lampamount = 5000 * costmply


bank = CFBank.CFBank(bankdatabase)

text = string.split(Crossfire.WhatIsMessage())


if text[0] == 'help' or text[0] == 'yes':
    message ='Commands are:\
    \n balance - shows your bank account balance \
    \n\n house - buy build scroll of house for %s,000,000 %ss \
    \n\n temple - buy build scroll of temple for %s,000,000 %ss \
    \n\n shop - buy build scroll of shop for %s,000,000 %ss \
    \n\n tob - buy build scroll of small tower for %s0,000,000 %ss \
    \n\n well - buy build scroll of well for %s00,000 %ss \
    \n\n fountain - buy build scroll of fountain for %s00,000 %ss \
    \n\n farm - buy build scroll of farm for %s00,000 %ss \
    \n\n prospect - buy build scroll of temporary prospect hole for %s0 %ss \
    \n\n mine - buy build scroll of mine for %s,000,000 %ss \
    \n\n harbor - buy build scroll of harbor for %s,000,000 %ss \
    \n\n guild - buy build scroll of small guild for %s,000,000 %ss \
    \n\n lamppost - buy build scroll of lamppost for %s,000 %ss \
    \n\nAll transactions are in %ss\n(1 : %s gold coins).' \
    %(costmply, curname0, tmplcostfnt, curname0, shopcostfnt, curname0, costmply, curname0, wellcostfnt, curname0, fountaincostfnt, curname0, farmcostfnt, curname0, costmply, curname0, minecostfnt, curname0, costmply, curname0, guildcostfnt, curname0, lampcostfnt, curname0, curname1, gexcrate)

elif text[0] == 'house':
    amount = houseamount
    if bank.withdraw(activatorname, amount):
        message = '%d %ss paid from bank account.' \
        %(amount, curname0)
        
        buildscroll = Crossfire.CreateObjectByName('scroll_2')
        buildscroll.Name = 'build scroll'
        buildscroll.NamePl = 'build scrolls'
        buildscroll.Title = 'of house'
        eapply = Crossfire.CreateObjectByName('event_apply')
        eapply.InsertInto(buildscroll)
        eapply.Name = 'house'
        eapply.Title = 'Python'
        eapply.Slaying = '/python/misc/build.py'
        buildscroll.InsertInto(activator)
    else:
        message = 'Not enough %ss on your account'%(curname1)

elif text[0] == 'temple':
    amount = templeamount
    if bank.withdraw(activatorname, amount):
        message = '%d %ss paid from bank account.' \
        %(amount, curname0)
        
        buildscroll = Crossfire.CreateObjectByName('scroll_2')
        buildscroll.Name = 'build scroll'
        buildscroll.NamePl = 'build scrolls'
        buildscroll.Title = 'of temple'
        eapply = Crossfire.CreateObjectByName('event_apply')
        eapply.InsertInto(buildscroll)
        eapply.Name = 'temple'
        eapply.Title = 'Python'
        eapply.Slaying = '/python/misc/build.py'
        buildscroll.InsertInto(activator)
    else:
        message = 'Not enough %ss on your account'%(curname1)

elif text[0] == 'shop':
    amount = shopamount
    if bank.withdraw(activatorname, amount):
        message = '%d %ss paid from bank account.' \
        %(amount, curname0)
        
        buildscroll = Crossfire.CreateObjectByName('scroll_2')
        buildscroll.Name = 'build scroll'
        buildscroll.NamePl = 'build scrolls'
        buildscroll.Title = 'of shop'
        eapply = Crossfire.CreateObjectByName('event_apply')
        eapply.InsertInto(buildscroll)
        eapply.Name = 'shop'
        eapply.Title = 'Python'
        eapply.Slaying = '/python/misc/build.py'
        buildscroll.InsertInto(activator)
    else:
        message = 'Not enough %ss on your account'%(curname1)

elif text[0] == 'tob':
    amount = tobamount
    if bank.withdraw(activatorname, amount):
        message = '%d %ss paid from bank account.' \
        %(amount, curname0)
        
        buildscroll = Crossfire.CreateObjectByName('scroll_2')
        buildscroll.Name = 'build scroll'
        buildscroll.NamePl = 'build scrolls'
        buildscroll.Title = 'of tob'
        eapply = Crossfire.CreateObjectByName('event_apply')
        eapply.InsertInto(buildscroll)
        eapply.Name = 'tob'
        eapply.Title = 'Python'
        eapply.Slaying = '/python/misc/build.py'
        buildscroll.InsertInto(activator)
    else:
        message = 'Not enough %ss on your account'%(curname1)

elif text[0] == 'well':
    amount = wellamount
    if bank.withdraw(activatorname, amount):
        message = '%d %ss paid from bank account.' \
        %(amount, curname0)

        buildscroll = Crossfire.CreateObjectByName('scroll_2')
        buildscroll.Name = 'build scroll'
        buildscroll.NamePl = 'build scrolls'
        buildscroll.Title = 'of well'
        eapply = Crossfire.CreateObjectByName('event_apply')
        eapply.InsertInto(buildscroll)
        eapply.Name = 'well'
        eapply.Title = 'Python'
        eapply.Slaying = '/python/misc/build.py'
        buildscroll.InsertInto(activator)
    else:
        message = 'Not enough %ss on your account'%(curname1)
	
elif text[0] == 'fountain':
    amount = fountainamount
    if bank.withdraw(activatorname, amount):
        message = '%d %ss paid from bank account.' \
        %(amount, curname0)

        buildscroll = Crossfire.CreateObjectByName('scroll_2')
        buildscroll.Name = 'build scroll'
        buildscroll.NamePl = 'build scrolls'
        buildscroll.Title = 'of fountain'
        eapply = Crossfire.CreateObjectByName('event_apply')
        eapply.InsertInto(buildscroll)
        eapply.Name = 'fountain'
        eapply.Title = 'Python'
        eapply.Slaying = '/python/misc/build.py'
        buildscroll.InsertInto(activator)
    else:
        message = 'Not enough %ss on your account'%(curname1)

elif text[0] == 'farm':
    amount = farmamount
    if bank.withdraw(activatorname, amount):
        message = '%d %ss paid from bank account.' \
        %(amount, curname0)

        buildscroll = Crossfire.CreateObjectByName('scroll_2')
        buildscroll.Name = 'build scroll'
        buildscroll.NamePl = 'build scrolls'
        buildscroll.Title = 'of farm'
        eapply = Crossfire.CreateObjectByName('event_apply')
        eapply.InsertInto(buildscroll)
        eapply.Name = 'farm'
        eapply.Title = 'Python'
        eapply.Slaying = '/python/misc/build.py'
        buildscroll.InsertInto(activator)
    else:
        message = 'Not enough %ss on your account'%(curname1)

elif text[0] == 'prospect':
    amount = prospectamount
    if bank.withdraw(activatorname, amount):
        message = '%d %ss paid from bank account.' \
        %(amount, curname0)

        buildscroll = Crossfire.CreateObjectByName('scroll_2')
        buildscroll.Name = 'build scroll'
        buildscroll.NamePl = 'build scrolls'
        buildscroll.Title = 'of prospect'
        eapply = Crossfire.CreateObjectByName('event_apply')
        eapply.InsertInto(buildscroll)
        eapply.Name = 'prospect'
        eapply.Title = 'Python'
        eapply.Slaying = '/python/misc/build.py'
        buildscroll.InsertInto(activator)
    else:
        message = 'Not enough %ss on your account'%(curname1)

elif text[0] == 'mine':
    amount = mineamount
    if bank.withdraw(activatorname, amount):
        message = '%d %ss paid from bank account.' \
        %(amount, curname0)

        buildscroll = Crossfire.CreateObjectByName('scroll_2')
        buildscroll.Name = 'build scroll'
        buildscroll.NamePl = 'build scrolls'
        buildscroll.Title = 'of mine'
        eapply = Crossfire.CreateObjectByName('event_apply')
        eapply.InsertInto(buildscroll)
        eapply.Name = 'mine'
        eapply.Title = 'Python'
        eapply.Slaying = '/python/misc/build.py'
        buildscroll.InsertInto(activator)
    else:
        message = 'Not enough %ss on your account'%(curname1)

elif text[0] == 'harbor':
    amount = harboramount
    if bank.withdraw(activatorname, amount):
        message = '%d %ss paid from bank account.' \
        %(amount, curname0)

        buildscroll = Crossfire.CreateObjectByName('scroll_2')
        buildscroll.Name = 'build scroll'
        buildscroll.NamePl = 'build scrolls'
        buildscroll.Title = 'of harbor'
        eapply = Crossfire.CreateObjectByName('event_apply')
        eapply.InsertInto(buildscroll)
        eapply.Name = 'harbor'
        eapply.Title = 'Python'
        eapply.Slaying = '/python/misc/build.py'
        buildscroll.InsertInto(activator)
    else:
        message = 'Not enough %ss on your account'%(curname1)

elif text[0] == 'guild':
    amount = guildamount
    if bank.withdraw(activatorname, amount):
        message = '%d %ss paid from bank account.' \
        %(amount, curname0)

        buildscroll = Crossfire.CreateObjectByName('scroll_2')
        buildscroll.Name = 'build scroll'
        buildscroll.NamePl = 'build scrolls'
        buildscroll.Title = 'of small guild'
        eapply = Crossfire.CreateObjectByName('event_apply')
        eapply.InsertInto(buildscroll)
        eapply.Name = 'guild'
        eapply.Title = 'Python'
        eapply.Slaying = '/python/misc/build.py'
        buildscroll.InsertInto(activator)
    else:
        message = 'Not enough %ss on your account'%(curname1)

elif text[0] == 'lamppost':
    amount = lampamount
    if bank.withdraw(activatorname, amount):
        message = '%d %ss paid from bank account.' \
        %(amount, curname0)
        
        buildscroll = Crossfire.CreateObjectByName('scroll_2')
        buildscroll.Name = 'build scroll'
        buildscroll.NamePl = 'build scrolls'
        buildscroll.Title = 'of lamppost'
        eapply = Crossfire.CreateObjectByName('event_apply')
        eapply.InsertInto(buildscroll)
        eapply.Name = 'lamp'
        eapply.Title = 'Python'
        eapply.Slaying = '/python/misc/build.py'
        buildscroll.InsertInto(activator)
    else:
        message = 'Not enough %ss on your account'%(curname1)
        
        
elif text[0] == 'balance':
    balance = bank.getbalance(activatorname)
    if balance == 1:
        message = 'Amount in bank: 1 %s'%(curname1)
    elif balance:
        message = 'Amount in bank: %d %ss'%(balance, curname1)
    else:
        message = 'Sorry, you have no balance.'


else:
    message = welcome

whoami.Say(message)
Crossfire.SetReturnValue(1)
