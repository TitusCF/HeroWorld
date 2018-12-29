import Crossfire
import CFLog

activator = Crossfire.WhoIsActivator()
name = activator.Name
ip = Crossfire.WhatIsMessage()

log = CFLog.CFLog()

if log.info(name):
    log.login_update(name, ip)
else:
    log.create(name)
