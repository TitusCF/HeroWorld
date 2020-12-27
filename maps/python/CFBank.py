"""
Created by: Joris Bontje <jbontje@suespammers.org>

This module stores bank account information.
"""

import os.path
import shelve

import Crossfire

class CFBank:
    def __init__(self, bankfile):
        self.bankdb_file = os.path.join(Crossfire.LocalDirectory(), bankfile)
        self.bankdb = shelve.open(self.bankdb_file)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()

    def deposit(self, user, amount):
        if not user in self.bankdb:
            self.bankdb[user] = amount
        else:
            balance = self.bankdb[user]
            self.bankdb[user] = balance + amount

    def withdraw(self, user, amount):
        if user in self.bankdb:
            balance = self.getbalance(user)
            if balance >= amount:
                self.bankdb[user] = balance - amount
                return 1
        return 0

    def getbalance(self, user):
        self._convert(user)
        if user in self.bankdb:
            return self.bankdb[user]
        else:
            return 0

    def remove_account(self, user):
        if user in self.bankdb:
            del self.bankdb[user]
            Crossfire.Log(Crossfire.LogDebug,
                          "%s's bank account removed." % user)
            return 1
        else:
            return 0

    def close(self):
        self.bankdb.close()

    def _convert(self, name):
        """Move a player's balance from the player file to the bank."""
        player = Crossfire.FindPlayer(name)
        if player is None:
            return 0
        old_balance = _balance_legacy(player)
        if old_balance > 0:
            Crossfire.Log(Crossfire.LogInfo,
                    "Converting bank account for %s with %d silver" \
                            % (name, old_balance))
            self.deposit(name, old_balance)
            player.WriteKey("balance", "moved-to-bank-file", 1)

def open():
    return CFBank('ImperialBank_DB')

def _balance_legacy(player):
    """Return the balance of the given player's bank account."""
    try:
        balance_str = player.ReadKey("balance")
        return int(balance_str)
    except ValueError:
        return 0
