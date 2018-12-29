# -*- coding: utf-8 -*-
# dialog_check.py
# This script is *not* intended to be used by the crossfire plugin, it is 
# designed to verify the correctness of a dialog script -independantly of the crossfire server.
# Typically you will want to run this script against a single file, the one that you specify in 
# the event_say, but if you want to check all of the .msg files in the map distribution, then
# you can run something like:
# for i in $(grep -h name.*msg ../../ -r | cut -d " " -f 2 | sort | uniq); do echo $i && python dialog_check.py "../../"$i; done
# from maps/python/dialog

import cjson
import sys
import os
import re

# There is an upper limit on the maximum message length the server can cope with, exceeding it throws out a warning.
MAX_MSG_LENGTH = 2048

def checkactionfile(filename, condition):
    args = condition[1:]
    checkstatus = 0
    if not os.path.isfile(filename):
        print "Error: No script to support action: ", condition[0], "Expected: ", filename
        return False
    else:
        actf = open(filename,"r")
        argnum = 0
        for actline in actf.readlines():
            if checkstatus == 0:
                if actline.find("## DIALOGCHECK") != -1:
                    checkstatus = 1
            elif checkstatus == 1:
                if actline.find("## ENDDIALOGCHECK") != -1:
                    checkstatus = 2
                elif actline.find("## MINARGS") != -1:
                    num = actline.split()[2]
                    if not num.isdigit():
                        print "ERROR: Action definition for script ", filename, " MINARGS not defined"
                        return False
                    else:
                        if len(args)<int(num):
                            print "ERROR: Insufficiant options passed to script ", filename, "expected ", int(num), " recieved ", len(args)
                            return False
                elif actline.find("## MAXARGS") != -1:
                    num = actline.split()[2]
                    if not num.isdigit():
                        print "ERROR: Action definition for script ", filename, " MAXARGS not defined"
                        return False
                    elif int(num) == 0:
                        # zero means there is no upper limit to the number of arguments
                        pass
                    else:
                        if len(args)>int(num):
                            print "ERROR: Too many options passed to script ", filename, "expected ", int(num), " recieved ", len(args)
                            return False
                elif actline.find("##") != 1:
                    # This is a regexp for one of the arguments
                    if argnum < len(args):
                        argmatch = re.compile(actline.split()[1])
                        if not argmatch.match(args[argnum]):
                            print "ERROR: Argument ", argnum+2, "of rule: ", condition, " doesn't match regexp ", actline.split()[1] 
                            return False
                    argnum+=1
        if checkstatus != 2:
            print "Warning: No dialogcheck block for file ", filename, " Unable to check condition ", condition
            return True
    return True
    
def checkdialoguefile(msgfile, location):

    rulenumber = 0
    errors = 0
    warnings = 0
    extrafiles =[]
    params = {}
    try:
        f = open(msgfile,"rb")
    except:
        print "ERROR: Can't open file, ", msgfile
        errors +=1 
    else:
        try:
            params = cjson.decode(f.read())
        except:
            print "ERROR: Failed to parse file, ", msgfile, "not a valid json file"
            errors +=1
        f.close()
    if "location" in params:
        if not location == '':
            print "Warning: Location defined multiple times in included files"
            warnings+=1
        location = params["location"]
    if location =='':
        print "Warning: no location was specified"
        warnings +=1
    rulenumber =0
    for jsonRule in params["rules"]:
        rulenumber +=1
        include = 0
        msg=0
        post=0
        match=0
        pre =0
        for action in jsonRule:
            if action == "pre":
                pre+=1 
                for condition in jsonRule["pre"]:
                    action = condition[0]
                    path = os.path.join("pre/", action + ".py")
                    if not checkactionfile(path, condition):
                        print "ERROR: verification of action file ", path, " failed for rule ", rulenumber, " condition ", condition
                        errors+=1
                    
            elif action == "msg":
                for line in jsonRule["msg"]:
                    if len(line) > MAX_MSG_LENGTH:
                        # We won't print out the entire line, because it's very 
                        # very long, but we'll print the first 70 characters in order to help identify it
                        print "WARNING: A Dialog Line for rule", rulenumber, "is too long. (", len(line), "characters, maximum is", MAX_MSG_LENGTH, ") \nLine begins:", line[:70]
                        warnings+=1
                msg+=1
            elif action == "post":
                post+=1
                for condition in jsonRule["post"]:
                    action = condition[0]
                    path = os.path.join("post/", action + ".py")
                    if not checkactionfile(path, condition):
                        print "ERROR: verification of action file ", path, " failed for rule ", rulenumber, " condition ", condition
                        errors +=1

            elif action == "match":
                match+=1
            elif action == "replies":
                pass
            elif action == "comment":
                pass
            elif action == "include":
                include+=1
                for condition in jsonRule["include"]:
                    if condition[0] == "/":
                        inclname = os.path.join("../..", condition[1:])
                    else:
                        inclname = os.path.join(os.path.dirname(msgfile), condition)
                    extrafiles.append(inclname)
            else:
                print "Warning: Ignoring unknown rule:", action
                warnings+=1
        if (include == 1 and msg+post+match == 0) or (msg == 1 and post == 1 and match ==1 and pre == 1):
            pass
        else:
            print "ERROR: Rule created with an invalid combination of actions, actions are: ", jsonRule.keys()
            errors +=1
    newfiles =0
    newrules =0
    newwarnings=0
    newerrors=0
    if len(extrafiles) > 0:
        for extrapath in extrafiles:
            newfiles, newrules, newwarnings, newerrors = checkdialoguefile(extrapath, location)
            print "checked ", newrules, "rules from file", extrapath, "Found ", newerrors, " errors and ", newwarnings,"warnings"
            warnings +=newwarnings
            rulenumber+=newrules
            errors+=newerrors
        extrafiles = []
    return (1+newfiles, rulenumber, warnings, errors)

if len(sys.argv) < 2:
    print "usage: python dialog_check.py path/to/dialogfile.msg"
    exit()
for arg in sys.argv[1:]:
    newfiles, rulecount, newwarnings, newerrors = checkdialoguefile(arg, '')
    print "checked ", rulecount, "rules from file", arg, "Found ", newerrors, " errors and ", newwarnings,"warnings"