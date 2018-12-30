#SlotMachine configuration file
#to make a new kind of slot machine, copy this file, change the settings and point the slotmachine to the new file.
#Special type Golden Galaxy wonder slot
#FYI - This one uses an object for cointype and not the money code :)

import Crossfire
import CFGamble
import CFItemBroker

activator=Crossfire.WhoIsActivator()
activatorname=activator.Name
whoami=Crossfire.WhoAmI()
#gets slot name and adds map name for unique jackpot
slotname= '%s#%s' %(whoami.Name,whoami.Map.Path)
x=activator.X
y=activator.Y

cointype = "igocert" #What type of coin is this slotmachine using?
coinname = "Gold Certificates"
minpot = 4000 #Minimum slot jackpot size
maxpot = 500000 #Maxiumum slot jackpot size
cost = 3 #Price of usage

#Change the items on the slot spinner or the number of items.
#slotlist = ["       ", "cherry ", "  bell ", " olive ", " * * * ", " $$ $$ ", "-B-A-R-", "2-BAR-2", "3-BAR-3", "7 7 7 7", "=G+N+U="]
slotlist = ["cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "cherry ", "  bell ", "  bell ", "  bell ", "  bell ", "  bell ", "  bell ", "  bell ", "  bell ", "  bell ", "  bell ", "  bell ", "  bell ", "  bell ", "  bell ", "  bell ", "  bell ", "  bell ", "  bell ", " olive ", " olive ", " olive ", " olive ", " olive ", " olive ", " olive ", " olive ", " olive ", " olive ", " olive ", " olive ", " olive ", " olive ", " * * * ", " * * * ", " * * * ", " * * * ", " * * * ", " * * * ", " * * * ", " * * * ", " * * * ", " * * * ", " $$ $$ ", " $$ $$ ", " $$ $$ ", " $$ $$ ", " $$ $$ ", " $$ $$ ", " $$ $$ ", " $$ $$ ", "-B-A-R-", "-B-A-R-", "-B-A-R-", "-B-A-R-", "-B-A-R-", "-B-A-R-", "2-BAR-2", "2-BAR-2", "2-BAR-2", "2-BAR-2", "3-BAR-3", "3-BAR-3", "3-BAR-3", "7 7 7 7", "7 7 7 7", "=G+N+U="]


spinners = 3 #How many spinners on the slotmachine?

Slots=CFGamble.SlotMachine(slotname,slotlist,minpot,maxpot)

object = activator.CheckInventory(cointype)
if (object):
    pay = CFItemBroker.Item(object).subtract(cost)
    if (pay):
       Slots.placebet(cost)
       results = Slots.spin(spinners)
       pay = 0
       pot = Slots.checkslot()
       activator.Write('%s' %results, 7)
       for item in results:
          #match all but one - pays out by coin e.g 3 to 1 or 4 to 1
          if results.count(item) == spinners:
             if item == "cherry ":
                pay = 10
             elif item == "  bell ":
                pay = 25
             elif item == " olive ":
                pay = 40
             else:
                break
             activator.Write("%d %ss, a minor win!" %(spinners,item))
             payoff = cost*pay
             Slots.payoff(payoff)
             id = activator.Map.CreateObject(cointype, x, y)
             CFItemBroker.Item(id).add(payoff)
             if payoff == 1:
                message = "you win %d %s!" %(payoff,coinname)
             else:
                message = "You win %d %ss!!" %(payoff,coinname)
             break
          elif results.count(item) == spinners:
             #all match - pays out by coin again
             activator.Write('%d %ss, a Major win!' %(spinners,item))
	     if item == " * * * ":
                pay = 62
             elif item == " $$ $$ ":
                pay = 80
             elif item == "-B-A-R-":
                pay = 150
             elif item == "2-BAR-2":
                pay = 300
             elif item == "3-BAR-3":
                pay = 1000
	     else:
                break
             activator.Write("%d %ss, a minor win!" %(spinners,item))
             payoff = cost*pay
             Slots.payoff(payoff)
             id = activator.Map.CreateObject(cointype, x, y)
             CFItemBroker.Item(id).add(payoff)
             if payoff == 1:
                message = "you win %d %s!" %(payoff,coinname)
             else:
                message = "You win %d %ss!!" %(payoff,coinname)
             break
          elif results.count(item) == spinners:
             #all match - pays out by coin again
             activator.Write('%d %ss, a Jackpot!' %(spinners,item))
             if item == "7 7 7 7":
                pay = 2000
             elif item == "=G+N+U=":
                pay = 10000
             #payoff = pot*pay
             payoff = cost*pay
	     Slots.payoff(payoff)
             id = activator.Map.CreateObject(cointype, x, y)
             CFItemBroker.Item(id).add(payoff)
             if payoff == 1:
                message = "you win %d %s!" %(payoff,coinname)
             else:
                message = "You win %d %ss!!" %(payoff,coinname)
             break
          else:
             message = "Better luck next time!"
       activator.Write(message)
       #activator.Write("%d in the Jackpot, Play again?" %Slots.checkslot())
    else:
       activator.Write("Sorry, you do not have enough %ss" %(coinname))
else:
   activator.Write("Sorry, you do not have any %ss" %(coinname))
