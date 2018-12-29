import Crossfire
import CFLog

activator = Crossfire.WhoIsActivator()
name = activator.Name

log = CFLog.CFLog()

log.logout_update(name)
