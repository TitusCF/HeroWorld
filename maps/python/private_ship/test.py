"""
Script run when we apply a 'testpython' object

This is NOT related to testing the private_ship functionalities, it is
just a playground to test for features.
"""

import Crossfire


log = lambda x: Crossfire.Log(Crossfire.LogInfo, repr(x))

log(Crossfire.DataDirectory())
log(Crossfire.MapDirectory())
log(Crossfire.UniqueDirectory())
log(Crossfire.LocalDirectory())

# do not apply the object
Crossfire.SetReturnValue(1)
