# Script for say event of Karen the receptionist
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

import CFPython
import sys
import traceback
sys.path.append('%s/%s/python' %(CFPython.GetDataDirectory(),CFPython.GetMapDirectory()))

# Constant price values in platinum
# As far as I can tell, the price calculations I use here are correct,
# but something dodgy happens to the actual prices in the game.
priceWritingPen=50
priceMailScroll=1  # Empty scrolls are worth less than one, but this price includes postage.

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
		message = 'How can I help you babe? \n- pen                (%s platinum)\n- scroll <friend> (%s platinum)\n- seen <friend>  (free)\n- deposit <kp>   (kp thousand platinum)\n- withdraw <kp> (free)\n- balance          (free)\n'%(priceWritingPen,priceMailScroll)
		CFPython.Say(whoami,message)

    elif text[0] == 'pen':
	if (CFPython.PayAmount(activator, priceWritingPen*50)):
		CFPython.Say(whoami, 'Here is your Ice castle Pen, honey.')
		id = CFPython.CreateObject('writing pen', (x, y))
		CFPython.SetName(id, 'Ice castle pen')
		CFPython.SetValue(id, priceWritingPen*50)
		CFPython.SetIdentified(id, 1);
	else:
		CFPython.Say(whoami, 'You need %s platinum for an Ice castle pen, sugar.'%priceWritingPen)

    elif text[0] == 'scroll':
	if len(text)==2:
		if log.exist(text[1]):
			if (CFPython.PayAmount(activator, priceMailScroll*50)):
				CFPython.Say(whoami, 'Here is your scroll, baby.')
				id = CFPython.CreateObject('scroll', (x, y))
				CFPython.SetName(id, 'mailscroll T: '+text[1]+' F: '+ activatorname)
				CFPython.SetValue(id, priceMailScroll*50)
				CFPython.SetIdentified(id, 1);
			else:
				CFPython.Say(whoami, 'Honey, you need %s platinum for a scroll.'%priceMailScroll)
		else:
			CFPython.Say(whoami, 'Sorry, I don\'t know any %s.'%text[1])

	else:
		CFPython.Say(whoami, 'Usage "scroll <friend>"')


    elif text[0] == 'warning':
	if (CFPython.IsDungeonMaster(activator)):
		if len(text)==2:
			if log.exist(text[1]):
				CFPython.Say(whoami, 'Here is your mailwarning sir.')
				id = CFPython.CreateObject('diploma', (x, y))
				CFPython.SetName(id, 'mailwarning T: '+text[1]+' F: '+ activatorname)
				CFPython.SetValue(id, priceMailScroll*50)
				CFPython.SetIdentified(id, 1);
			else:
				CFPython.Say(whoami, 'Sorry, I don\'t know any %s.'%text[1])

		else:
			CFPython.Say(whoami, 'Usage "warning <foo>"')
	else:
		CFPython.Say(whoami, 'You need to be DM to be able to use this command, sugar.')

    elif text[0] == 'seen':
	if len(text)==2:
		if log.exist(text[1]):
			ip, date, count = log.info(text[1])
			CFPython.Say(whoami, "I have seen %s %d times, last time from %s on %s." % (text[1], count, ip, date))
		else:
			CFPython.Say(whoami, "Sorry, I have never seen '%s'." % text[1])
	else:
		CFPython.Say(whoami, 'Usage "seen <friend>"')

    elif text[0] == 'deposit':
	if len(text)==2:
		if (CFPython.PayAmount(activator, int(text[1])*50*1000)):
			log.deposit(activatorname, int(text[1]))
			CFPython.Say(whoami, 'Deposited to bank account.')
		else:
			CFPython.Say(whoami, 'You need %d platinum.'%(int(text[1])*1000))
	else:
		CFPython.Say(whoami, 'Usage "deposit <kp>" (kp = kilo platinum)')

    elif text[0] == 'withdraw':
	if len(text)==2:
		if (log.withdraw(activatorname, int(text[1]))):
			CFPython.Say(whoami, 'Don\'t spend it all at once.')
			id = CFPython.CreateObject('platinum coin', (x, y))
			CFPython.SetQuantity(id, int(text[1])*1000)
		else:
			CFPython.Say(whoami, 'You are not that rich.')
	else:
		CFPython.Say(whoami, 'Usage "withdraw <kp>" (kp = kilo platinum)')

    elif text[0] == 'balance':
	balance = log.getbalance(activatorname)
	CFPython.Say(whoami, 'Amount in bank: %d kilo platinum'%balance)
	if balance>9999:
	    CFPython.Say(whoami, 'Can I have your baby?')
	elif balance>9999:
	    CFPython.Say(whoami, 'Marry me?')
	elif balance>999:
	    CFPython.Say(whoami, 'How about dinner and catch the late show at the theater?')
	elif balance>99:
	    CFPython.Say(whoami, 'Have I mentioned that you are very attractive?')

    else:
	CFPython.Say(whoami, 'Honey, can I help you?')

except:
    CFPython.Write('Receptionist Error :', activator)
    #for line in traceback.format_exception(sys.exc_type, sys.exc_value, sys.exc_info()[2]):
	#CFPython.Write(line, activator)

    raise
