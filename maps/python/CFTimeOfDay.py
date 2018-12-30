import Crossfire

class TimeOfDay:
    def __init__(self):
        self.now = Crossfire.GetTime()
        self.current = [Crossfire.GetMonthName(self.now[1]),Crossfire.GetWeekdayName(self.now[5]),Crossfire.GetSeasonName(self.now[7]),Crossfire.GetPeriodofdayName(self.now[8])]

    def matchAny(self,what):
        if isinstance(what,list):
            return bool(set(what) & set(self.current))
        else:
            return bool(set([what]) & set (self.current))

    def matchAll(self,what):
        if isinstance(what,list):
            return bool(not (set(what) - set(self.current)))
        else:
            return bool(not (set([what]) - set (self.current)))
    def log(self):
        Crossfire.Log(Crossfire.LogDebug,"current time is seen as %s" %self.current)
