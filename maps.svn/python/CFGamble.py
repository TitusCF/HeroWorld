#CFGamble
# Todd Mitchell
#The Python control file for Slot Machines and other such nonsense
#Please do not put CFPython functions in this file,
#but rather place these in the calling file (don't ask me why - it just feels right)

import os.path
import shelve
import random

import Crossfire

class SlotMachine:
	#sets up the file that holds all the slotmachine jackpots
	#make sure this points to your writable var/crossfire directory
	#you can delete that file to reset all the slotmachine jackpots
	slotfile = os.path.join(Crossfire.LocalDirectory(),'SlotMachine_file')
	slotdb = {}
	def __init__(self,slotname,slotlist,minpot,maxpot):
		slotdb = shelve.open(self.slotfile)
		self.slotname = slotname
		self.slotlist = slotlist
		self.minpot = minpot
		self.maxpot = maxpot

	def placebet(self,amount):
		if self.slotname not in self.slotdb:
			self.slotdb[self.slotname] = self.minpot+amount
		else:
			temp=self.slotdb[self.slotname]
			self.slotdb[self.slotname]=temp+amount

	def payoff(self,amount):
		temp=self.slotdb[self.slotname]
		self.slotdb[self.slotname] = temp-amount

	def spin(self,slotnum):
		result=[]
		while slotnum >=1:
			r = self.slotlist[random.randint(0,len(self.slotlist)-1)]
			result.append(r)
			slotnum=slotnum-1
		return result

	def checkslot(self):
		limit = self.slotdb[self.slotname]
		if limit >= self.maxpot:
			self.slotdb[self.slotname] = self.maxpot
		elif limit < self.minpot:
			self.slotdb[self.slotname] = self.minpot
		return self.slotdb[self.slotname]
