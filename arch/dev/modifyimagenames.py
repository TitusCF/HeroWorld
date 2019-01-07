import os.path,sys,re,string

reverse_map = {}
filenames = []
reverse_map2 = {}
filenames2 = []


matcher = re.compile("\..*")

def foreachimage(arg, dirname, names):
    #print dirname, ":"
    global filenames

    for n in names[:]: # don't touch dot files
        if matcher.match(n):
            names.remove(n)

    kill_list = ['.svn', 'README', 'dev', 'unused', 'obsolete', 'retired']
    for i in kill_list:
        try:    names.remove(i)
        except: pass

    #print names
    for n in names:
        fname = os.path.join(dirname,n)
        #print "'%s'" % fname
        if os.path.isfile(fname) and re.match(".*\.png", fname):
            global reverse_map
            #if(reverse_map.has_key(n)):
                #print "duplicate: ", fname
            reverse_map[n] = dirname
            #print "\t", fname
            filenames.append(n)


os.path.walk(sys.argv[1], foreachimage, "")

def foreachotherimage(arg, dirname, names2):
    #print dirname, ":"
    global filenames2

    for n in names2[:]: # don't touch dot files
        if matcher.match(n):
            names2.remove(n)

    kill_list = ['.svn', 'README', 'dev', 'unused', 'obsolete', 'retired']
    for i in kill_list:
        try:    names2.remove(i)
        except: pass

    #print names2
    for n in names2:
        fname = os.path.join(dirname,n)
        #print "'%s'" % fname
        if os.path.isfile(fname) and re.match(".*\.png", fname):
            global reverse_map2
            #if(reverse_map2.has_key(n)):
                #print "duplicate: ", fname
            reverse_map2[n] = dirname
            filenames2.append(n)
            #print "\t", fname

os.path.walk(sys.argv[2], foreachotherimage, "")
for n in filenames:
    if(reverse_map2.has_key(n)):
        newname = re.match("(^.*\.)(.*\.)(png)", n)
        if not newname:
            print "error: n doesn't match"
        if newname:
            #print newname.group(1)
            #print newname.group(0)
            newname = newname.group(1) + sys.argv[3] + "." + newname.group(2) + newname.group(3)
            #print newname
            #newname = n[:-8]
            #newname = newname +sys.argv[3]+n[-8:-4]+".png"
            command = "mv %s %s" % (os.path.join(reverse_map2[n], n)
                                , os.path.join(reverse_map[n], newname))
            print command
            #os.system(command)




