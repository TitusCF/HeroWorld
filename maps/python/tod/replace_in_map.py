import Crossfire
import random
from CFMapTransformer import CFMapTransformer
from CFTimeOfDay import TimeOfDay
import cjson
import string


event = Crossfire.WhatIsEvent()
alreadymatched = (event.Value!=0)
parameters = cjson.decode(event.Message)
current = TimeOfDay()
#current.log()
inverse = "inverse" in parameters and parameters["inverse"] == True
match = False
if not "match" in parameters:
    Crossfire.Log(Crossfire.LogError,"Script replace_in_map_period.py didn't get a 'match' parameter. Only got %s" %parameters)
elif parameters["match"].lower() == "one":
    match=TimeOfDay().matchAny(parameters["when"]) != inverse
elif parameters["match"].lower() == "all":
    match=TimeOfDay().matchAll(parameters["when"]) != inverse
else:
    Crossfire.Log(Crossfire.LogError,"Script replace_in_map_period.py didn't get a 'match' parameter. Only got %s" %parameters)

#print "match is %s and alreadymatched is %s" %(match,alreadymatched)

if (match != alreadymatched):
    transformer = CFMapTransformer(parameters["key"])
    if (match):
        Crossfire.Log(Crossfire.LogDebug,"Transforming %s into %s" %(parameters["from"],parameters["to"]))
        transformer.transformAll(parameters["from"],parameters["to"])
        event.Value=1
    else:
        transformer.untransformAll()
        event.Value=0

#if random.randint(0,2) == 0 :
#    print "transform mode"
#    transformer.transformAll("skeleton",["bones1","bones2","bones3","bones4"])
#else:
#    print "untransform mode"
#    transformer.untransformAll()
