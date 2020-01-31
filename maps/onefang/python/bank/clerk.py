# Script for say event of IPO employees 
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
# help                - gives information about usage
# pen                 - drops IPO Writing Pen on the floor
# literacy            - drops IPO Scroll of Literacy on the floor
# mailscroll <friend> - drops mailscroll to <friend> on the floor
# mailwarning <foo>   - drops mailwarning to <foo> on the floor

import CFPython
import sys
import traceback
sys.path.append('%s/%s/python' %(CFPython.GetDataDirectory(),CFPython.GetMapDirectory()))

# Constant price values in platinum
# As far as I can tell, the price calculations I use here are correct,
# but something dodgy happens to the actual prices in the game.
priceWritingPen=100
priceScrollOfLiteracy=30
priceMailScroll=2        # Empty scrolls are worth less than one, but this price includes postage.

activator = CFPython.WhoIsActivator()

try:
    import string
    import OFBank

    activatorname=CFPython.GetName(activator)
    whoami=CFPython.WhoAmI()
    x=CFPython.GetXPosition(activator)
    y=CFPython.GetYPosition(activator)

    log = OFBank.OFBank()
    text = string.split(CFPython.WhatIsMessage())

    if len(text)==0:
	text=['']

    if text[0] == 'help' or text[0] == 'yes':
		message = 'How can I help you?  Here is a list of commands:\n- pen                (%s platinum)\n- literacy           (%s platinum)\n- scroll <friend> (%s platinum)\n- seen <friend>  (free)\n- deposit <kp>   (kp thousand platinum)\n- withdraw <kp> (free)\n- balance          (free)\n'%(priceWritingPen,priceScrollOfLiteracy,priceMailScroll)
		CFPython.Say(whoami,message)

    elif text[0] == 'pen':
	if (CFPython.PayAmount(activator, priceWritingPen*50)):
		log.profit(priceWritingPen*50/2)
		CFPython.Say(whoami, 'Here is your IPO Writing Pen')
		id = CFPython.CreateObject('writing pen', (x, y))
		CFPython.SetName(id, 'IPO Writing Pen')
		CFPython.SetValue(id, priceWritingPen*50)
	else:
		CFPython.Say(whoami, 'You need %s platinum for an IPO Writing Pen'%priceWritingPen)

    elif text[0] == 'literacy':
	if (CFPython.PayAmount(activator,priceScrollOfLiteracy*50)):
		log.profit(priceScrollOfLiteracy*50/2)
        	CFPython.Say(whoami, 'Here is your IPO Scroll of Literacy')
        	id = CFPython.CreateObject('scroll of literacy', (x, y))
		CFPython.SetName(id, 'IPO Scroll of Literacy')
		CFPython.SetValue(id, priceScrollOfLiteracy*50)
	else:
		CFPython.Say(whoami, 'You need %s platinum for an IPO Scroll of Literacy'%priceScrollOfLiteracy)


    elif text[0] == 'mailscroll':
	if len(text)==2:
		if log.exist(text[1]):
			if (CFPython.PayAmount(activator, priceMailScroll*50)):
				log.profit(priceMailScroll*50/2)
				CFPython.Say(whoami, 'Here is your mailscroll')
				id = CFPython.CreateObject('scroll', (x, y))
				CFPython.SetName(id, 'mailscroll T: '+text[1]+' F: '+ activatorname)
				CFPython.SetValue(id, priceMailScroll*50)
			else:
				CFPython.Say(whoami, 'You need %s platinum for a mailscroll'%priceMailScroll)
		else:
			CFPython.Say(whoami, 'I don\'t know any %s'%text[1])

	else:
		CFPython.Say(whoami, 'Usage "mailscroll <friend>"')


    elif text[0] == 'mailwarning':
	if (CFPython.IsDungeonMaster(activator)):
		if len(text)==2:
			if log.exist(text[1]):
				CFPython.Say(whoami, 'Here is your mailwarning')
				id = CFPython.CreateObject('diploma', (x, y))
				CFPython.SetName(id, 'mailwarning T: '+text[1]+' F: '+ activatorname)
				CFPython.SetValue(id, 0)
			else:
				CFPython.Say(whoami, 'I don\'t know any %s'%text[1])

		else:
			CFPython.Say(whoami, 'Usage "mailwarning <foo>"')
	else:
		CFPython.Say(whoami, 'You need to be DM to be able to use this command')


    elif text[0] == 'seen':
	if len(text)==2:
		if log.exist(text[1]):
			ip, date, count = log.info(text[1])
			CFPython.Say(whoami, "I have seen '%s' joining %d times, last at %s, using IP: %s" % (text[1], count, date, ip))
		else:
			CFPython.Say(whoami, "I have never seen '%s' joining" % text[1])
	else:
		CFPython.Say(whoami, 'Usage "seen <friend>"')

    elif text[0] == 'deposit':
	if len(text)==2:
		if (CFPython.PayAmount(activator, int(text[1])*50*1000)):
			log.deposit(activatorname, int(text[1]))
			CFPython.Say(whoami, 'Deposited to bank account')
		else:
			CFPython.Say(whoami, 'You need %d platinum'%(int(text[1])*1000))
	else:
		CFPython.Say(whoami, 'Usage "deposit <amount kp>"')

    elif text[0] == 'withdraw':
	if len(text)==2:
		if (log.withdraw(activatorname, int(text[1]))):
			log.profit(50)
			CFPython.Say(whoami, 'Withdrawn from bank account')
			id = CFPython.CreateObject('platinum coin', (x, y))
			CFPython.SetQuantity(id, (int(text[1]) - 1)*1000)
		else:
			CFPython.Say(whoami, 'Not enough kp on your account')
	else:
		CFPython.Say(whoami, 'Usage "withdraw <amount kp>"')

    elif text[0] == 'balance':
	balance = log.getbalance(activatorname)
	CFPython.Say(whoami, 'Amount on bank: %d kp'%balance)

    else:
	CFPython.Say(whoami, 'Do you need help?')

except:
    CFPython.Write('Clerk Error :', activator)
    #for line in traceback.format_exception(sys.exc_type, sys.exc_value, sys.exc_info()[2]):
	#CFPython.Write(line, activator)

    raise
