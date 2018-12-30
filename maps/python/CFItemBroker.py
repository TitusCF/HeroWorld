#CFItemBroker.py
#An often used bit of code to add or remove a number of objects
#Useful for removing items (like in payment or as part of
#an inventory check)  This is also useful for setting the number
#of a newly created item(s) as it will check for existing item(s) and add
#the appropriate number of new items avoiding such silliness as the
#number of the existing items being reset.
#This will not check for the existence of an item as that would better
#be done in the calling script so you can be flexible.
#
#ToddMitchell

import Crossfire

class Item:

    def __init__(self, object):
        self.object = object
        self.numberof = self.object.Quantity
        # 0 for non merging items
        if self.numberof == 0:
            self.numberof = 1

    def add(self, number):
        tmp = (self.numberof + number)-1
        self.object.Quantity=tmp
        return 1

    def subtract(self, number):
        remainder = self.numberof - number
        if remainder >= number:
            self.object.Quantity=remainder
            return 1
        elif remainder == 0:
            self.object.Remove()
            return 1
        else:
            return 0

    def quantity(self):
        return self.numberof
