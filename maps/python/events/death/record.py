import CFDataFile
import Crossfire

death_file = CFDataFile.CFData("permadeaths", ["name"])
op = Crossfire.WhoAmI()
Crossfire.Log(Crossfire.LogDebug, "%s died permanently" % op.Name)
death_file.put_record({'#': op.Name, 'name': op.Name})
