# ship.py -- sailing around the world takes time
# Created by: Kevin Zheng <kevinz5000@gmail.com>
#
# This script should be used in specially designed ship maps. Please see the
# ship from Scorn to Santo Dominion for a working example. Note that connection
# 10 and 11 must be used for the entrance and exit gates, respectively.
import Crossfire

player = Crossfire.WhoIsActivator()
whoami = Crossfire.WhoAmI()
sailtime = 15

# Get a previously set value, otherwise return zero.
def getValue(key):
    value = Crossfire.WhoAmI().ReadKey(key)

    if (len(value) != 0):
        return value
    else:
        return 0

# Set a value.
def setValue(key, value):
    Crossfire.WhoAmI().WriteKey(key, str(value), 1)

# Check ship state.
def getState():
    return int(getValue("ship_state"))

# Set ship state.
def setState(state):
    setValue("ship_state", state)

# Check if the ship is already sailing.
def isSailing():
    if getState() != 0:
        return 1
    else:
        return 0

# Set a timer for the specified amount of time.
def setTimer(time):
    setValue("ship_timer", str(whoami.CreateTimer(time, 1)))

# Clear the previously set timer.
def clearTimer():
    Crossfire.DestroyTimer(int(getValue("ship_timer")))

# Start sailing if the ship isn't already sailing.
if Crossfire.WhatIsEvent().Subtype == Crossfire.EventType.APPLY:
    if isSailing() == 0:
        setState(1)

        # Close the entrance and notify the players onboard.
        whoami.Map.TriggerConnected(10, 1)
        whoami.Map.Print("\"Ahoy, the ship is ready to set sail!\" You feel the ship beginning to move.")
        setTimer(sailtime)
    else:
        player.Message("The ship has already set sail, be patient!")

# Handle the timer event based on what state the ship is in.
elif Crossfire.WhatIsEvent().Subtype == Crossfire.EventType.TIMER:
    if getState() == 1:
        # Open the exit and tell players that they've arrived.
        setState(2)
        whoami.Map.TriggerConnected(11, 1)
        whoami.Map.Print("The ship has arrived at its destination.")

        # Clear and reset timer for another 15 seconds.
        clearTimer()
        setTimer(15)
    elif getState() == 2:
        # Reset the ship.
        clearTimer()
        whoami.Map.TriggerConnected(10, 0)
        whoami.Map.TriggerConnected(11, 0)
        whoami.Map.Print("The ship is ready to board.")
        setState(0)
