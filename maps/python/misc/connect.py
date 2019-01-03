# -*- coding: utf-8 -*-
# Simple script to trigger the connection given as a parameter.
# Setting the script option to "3" will turn the connection 3 on in the map it was called from.

import Crossfire

if Crossfire.ScriptParameters().isdigit():
    Crossfire.WhoAmI().Map.TriggerConnected(int(Crossfire.ScriptParameters()), 1)
