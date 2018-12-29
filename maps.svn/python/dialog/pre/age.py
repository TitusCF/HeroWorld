# -*- coding: utf-8 -*-
#age.py
# This is one of the files that can be called by an npc_dialog, 
# The following code runs when a dialog has a pre rule of 'age'
# The syntax is 
# ["age", "agetoken", "years", "months", "days", "hours", "minutes"]
# To deliver a True verdict, the agetoken must correspond to a date 
# that is was at least as long ago as the duration specified in the options.
## DIALOGCHECK
## MINARGS 6
## MAXARGS 6
## .*
## \d
## \d
## \d
## \d
## \d
## ENDDIALOGCHECK


class checkfailed(Exception):
    pass

# maximum months, days, hours, as defined by the server.
# minutes per hour is hardcoded to approximately 60

try:
    MAXTIMES = [Crossfire.Time.MONTHS_PER_YEAR, Crossfire.Time.WEEKS_PER_MONTH*Crossfire.Time.DAYS_PER_WEEK, 
                Crossfire.Time.HOURS_PER_DAY, 60]
    # we have three times to consider, the old time, the current time, and the desired time difference.
    if len(args) != 6:
        raise checkfailed()
    markername = args[0]
    oldtime = self.getStatus(markername).split("-")
    oldtime = map(int, oldtime)
    if len(oldtime) !=5:
        # The marker hasn't been set yet
        raise checkfailed()

    desireddiff = map(int, args[1:])
    currenttime = (Crossfire.GetTime())[:5]
    actualdiff = []

    for i in range(5):
        actualdiff.append(currenttime[i]-oldtime[i])
        
    for i in range(4,0,-1):
    # let's tidy up desireddiff first
        if desireddiff[i] > MAXTIMES[i-1]:
            desireddiff[i-1] += desireddiff[i] // MAXTIMES[i-1]
            desireddiff[i] %= MAXTIMES[i-1]
    # Then actualdiff
        if actualdiff[i] < 0:
            actualdiff[i] += MAXTIMES[i-1]
            actualdiff[i-1] -=1
    Crossfire.Log(Crossfire.LogDebug, "CFDialog: tidied up desired difference: %s actual difference %s" %(desireddiff, actualdiff))
    for i in range(5):
        if actualdiff[i] < desireddiff[i]:
            raise checkfailed()     
except checkfailed:
    verdict = False