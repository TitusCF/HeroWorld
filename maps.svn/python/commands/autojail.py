import Crossfire

who = Crossfire.WhoAmI()

if who.DungeonMaster:
    value = Crossfire.ScriptParameters()
    dict = Crossfire.GetSharedDictionary()
    if value == '1':
        dict['autojail'] = 1
        who.Message('Autojail enabled')
    elif value == '0':
        dict['autojail'] = 0
        who.Message('Autojail disabled')
    else:
        autojail = 0
        if 'autojail' in dict and dict['autojail'] == 1:
            autojail = 1
        who.Message('Autojail is %d'%autojail)
