"""
Created by: Joris Bontje <jbontje@suespammers.org>

This module implements banking in Crossfire. It provides the 'say' event for
bank tellers.
"""

import random

import Crossfire
import CFBank
import CFItemBroker

activator = Crossfire.WhoIsActivator()
whoami = Crossfire.WhoAmI()
x = activator.X
y = activator.Y

# Associate coin names with their corresponding values in silver.
CoinTypes = {
    'SILVER': 1,
    'GOLD': 10,
    'PLATINUM': 50,
    'JADE': 5000,
    'AMBERIUM': 500000,
    'IMPERIAL NOTE': 10000,
    '10 IMPERIAL NOTE': 100000,
    '100 IMPERIAL NOTE': 1000000,
}

# Associate coin names with their corresponding archetypes.
ArchType = {
    'SILVER': 'silvercoin',
    'GOLD': 'goldcoin',
    'PLATINUM': 'platinacoin',
    'JADE': 'jadecoin',
    'AMBERIUM': 'ambercoin',
    'IMPERIAL NOTE': 'imperial',
    '10 IMPERIAL NOTE': 'imperial10',
    '100 IMPERIAL NOTE': 'imperial100',
}

# Define several 'thank-you' messages which are chosen at random.
thanks_message = [
    'Thank you for banking the Imperial Way.',
    'Thank you for banking the Imperial Way.',
    'Thank you, please come again.',
    'Thank you, please come again.',
    'Thank you for your patronage.',
    'Thank you for your patronage.',
    'Thank you, have a nice day.',
    'Thank you, have a nice day.',
    'Thank you. "Service" is our middle name.',
    'Thank you. "Service" is our middle name.',
    'Thank you. Hows about a big slobbery kiss?',
]

def getCoinNameFromArgs(argv):
    """Piece together several arguments to form a coin name."""
    coinName = str.join(" ", argv)
    # Remove the trailing 's' from the coin name.
    if coinName[-1] == 's':
        coinName = coinName[:-1]
    return coinName

def getExchangeRate(coinName):
    """Return the exchange rate for the given type of coin."""
    if coinName.upper() in CoinTypes:
        return CoinTypes[coinName.upper()]
    else:
        return None

def do_deposit(player, amount):
    """Deposit the given amount for the player."""
    with CFBank.open() as bank:
        bank.deposit(player.Name, amount)
        whoami.Say("%s credited to your account." \
                % Crossfire.CostStringFromValue(amount))

def cmd_help():
    """Print a help message for the player."""
    whoami.Say("The Bank of Skud can help you keep your money safe. In addition, you will be able to access your money from any bank in the world! What would you like to do?")

    Crossfire.AddReply("coins", "I'd like to know more about existing coins.")
    Crossfire.AddReply("balance", "I want to check my balance.")
    Crossfire.AddReply("deposit", "I'd like to deposit some money.")
    Crossfire.AddReply("withdraw", "I'd like to withdraw some money.")
    Crossfire.AddReply("convert", "I'd like to compute money conversions.")

def cmd_balance(argv):
    """Find out how much money the player has in his/her account."""
    balance = 0
    with CFBank.open() as bank:
        balance = bank.getbalance(activator.Name)
    if len(argv) >= 2:
        # Give balance using the desired coin type.
        coinName = getCoinNameFromArgs(argv[1:])
        exchange_rate = getExchangeRate(coinName)
        if exchange_rate is None:
            whoami.Say("Hmm... I've never seen that kind of money.")
            return
        if balance != 0:
            balance /= exchange_rate * 1.0
            whoami.Say("You have %.3f %s in the bank." % (balance, coinName))
        else:
            whoami.Say("Sorry, you have no balance.")
    else:
        whoami.Say("You have %s in the bank." % \
                Crossfire.CostStringFromValue(balance))

def cmd_deposit(text):
    """Deposit a certain amount of money."""
    if len(text) >= 3:
        coinName = getCoinNameFromArgs(text[2:])
        exchange_rate = getExchangeRate(coinName)
        amount = int(text[1])
        if exchange_rate is None:
            whoami.Say("Hmm... I've never seen that kind of money.")
            return
        if amount < 0:
            whoami.Say("Regulations prohibit negative deposits.")
            return

        # Make sure the player has enough cash on hand.
        actualAmount = amount * exchange_rate
        if activator.PayAmount(actualAmount):
            do_deposit(activator, actualAmount)
        else:
            whoami.Say("But you don't have that much in your inventory!")
    else:
        whoami.Say("What would you like to deposit?")
        Crossfire.AddReply("deposit <amount> <coin type>", "Some money.")

def cmd_withdraw(argv):
    """Withdraw money from the player's account."""
    if len(argv) >= 3:
        coinName = getCoinNameFromArgs(argv[2:])
        exchange_rate = getExchangeRate(coinName)
        amount = int(argv[1])
        if exchange_rate is None:
            whoami.Say("Hmm... I've never seen that kind of money.")
            return
        if amount <= 0:
            whoami.Say("Sorry, you can't withdraw that amount.")
            return

        # Make sure the player has sufficient funds.
        with CFBank.open() as bank:
            if bank.withdraw(activator.Name, amount * exchange_rate):
                message = "%d %s withdrawn from your account. %s" \
                        % (amount, coinName, random.choice(thanks_message))

                # Drop the money and have the player pick it up.
                withdrawal = activator.Map.CreateObject(
                        ArchType.get(coinName.upper()), x, y)
                CFItemBroker.Item(withdrawal).add(amount)
                activator.Take(withdrawal)
            else:
                message = "I'm sorry, you don't have enough money."
    else:
        message = "How much money would you like to withdraw?"
        Crossfire.AddReply("withdraw <amount> <coin name>", "This much!")

    whoami.Say(message)

def cmd_convert(argc):

  def attempt(argc):
    if len(argc) < 4 or argc[3] != "to":
      return False

    fromCoin = getCoinNameFromArgs(argc[2:3])
    toCoin = getCoinNameFromArgs(argc[4:5])
    if not toCoin or not fromCoin:
      return False

    fromValue = getExchangeRate(fromCoin)
    toValue = getExchangeRate(toCoin)

    fromAmount = int(argc[1])
    amount = int(fromAmount * fromValue / toValue)
    whoami.Say("{} {} coin{} would be {} {} coin{}".format(fromAmount, fromCoin, "s" if fromAmount > 1 else "", amount, toCoin, "s" if amount > 1 else ""))
    return True

  if not attempt(argc):
    whoami.Say("Sorry, I don't understand what you want to convert.\nTry something like \"4 platinum to silver\" please.")

def cmd_coins(_):
  whoami.Say("""The smallest coin available is the silver coin.
The gold coin is worth 10 silver coins, while the platinum coin is worth 50.
A jade coin is worth 5000 silver, and an amberium 500000.

There also exist imperial notes, worth 10000 silver coins.
""")

def main_employee():
    text = Crossfire.WhatIsMessage().split()
    if text[0] == "learn":
        cmd_help()
    elif text[0] == "balance":
        cmd_balance(text)
    elif text[0] == "deposit":
        cmd_deposit(text)
    elif text[0] == "withdraw":
        cmd_withdraw(text)
    elif text[0] == "convert":
        cmd_convert(text)
    elif text[0] == "coins":
        cmd_coins(text)
    else:
        whoami.Say("Hello, what can I help you with today?")
        Crossfire.AddReply("learn", "I want to learn how to use the bank.")

Crossfire.SetReturnValue(1)
try:
    main_employee()
except ValueError:
    whoami.Say("I don't know how much money that is.")
