import Crossfire

dict = Crossfire.GetSharedDictionary()

if 'autojail' in dict and dict['autojail'] == 1:
    activator = Crossfire.WhoIsActivator()
    activator.Message("Notice: killing another player will automatically send you to jail.")
