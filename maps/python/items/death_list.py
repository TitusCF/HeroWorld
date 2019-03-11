import CFDataFile
import Crossfire

death_file = CFDataFile.CFData("permadeaths", ["name"])
op = Crossfire.WhoIsActivator()
op.Message("Here is a list of those who have permanently died:")
for name in death_file.get_keys():
    op.Message(name)
Crossfire.SetReturnValue(1)
