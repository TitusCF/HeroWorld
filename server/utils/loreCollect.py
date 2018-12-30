#loreCollect.py
#To collect lore - endlore contents from Crossfire maps and archetypes file
#temitchell@sympatici.ca for comments/requests
#Python 2.x (never tested on 1.5...)

import sys
import os
import string

def mapLoreCollect(lfile, dir, files):
    for file in files:
        file = os.path.join(dir,file)
        try:
            f = open(file,'r')
            contents = f.read().split('\n')
            match = 0
            for line in contents:
                if line == 'maplore':
                    lfile.write("lore\n")
                    match = 1
                elif match == 1 and line == 'lore':
                    match = 2
                elif match == 2 and line == 'endmaplore':
			match = 0
			break
                elif match == 2:
                    lfile.write('%s\n' %(line))
                else:
                    pass
            f.close()
        except (OSError, IOError):
            pass

def loreCollect(file, lfile):
        try:
	    f = open(file,'r')
            contents = f.read().split('\n')
            match = 0
            for line in contents:
                if line[:5] == 'name ':
                    tempname = line
                    match = 1
                elif match == 1 and line == 'lore':
                    match = 2
                    lfile.write("lore\n%s\n" %tempname)
                    tempname = 0
                elif match == 2 and line == 'endlore':
                    match = 0
                    lfile.write("endlore\n")
                    pass
                elif match == 2:
                    lfile.write('%s\n' %(line))
                else:
                    pass
            f.close()
        except (OSError, IOError):
	    print "Error"
            pass

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 4:
        sys.stderr.write ('Collects lore from maps and arches into a single file\nUsage: loreCollect.py <map directory root> <path to archetypes file> <target filename>')
        sys.exit()
    else:
        lfile = open(sys.argv[3],'w')
	print "Collecting map lore...this may take a minute"
        os.path.walk(sys.argv[1],mapLoreCollect,lfile)
        print "Collecting arch lore..."
	loreCollect(sys.argv[2],lfile)
        lfile.close()
        print "finished collecting lore"
