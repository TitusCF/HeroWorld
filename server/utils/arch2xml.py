#arch2xml - Todd Mitchell - work in progress - use if you like
#usage : python arch2xml.py <directory> <xml file name>
#FYI - the Walk function is very useful for other python programs as well

import fnmatch, os, string

#Flexable Directory Walker from the Python CookBook

def Walk( root, recurse=0, pattern='*', return_folders=0 ):


	# initialize
	result = []

	# must have at least root folder
	try:
		names = os.listdir(root)
	except os.error:
		return result

	# expand pattern
	pattern = pattern or '*'
	pat_list = string.splitfields( pattern , ';' )

	# check each file
	for name in names:
		fullname = os.path.normpath(os.path.join(root, name))

		# grab if it matches our pattern and entry type
		for pat in pat_list:
			if fnmatch.fnmatch(name, pat):
				if os.path.isfile(fullname) or (return_folders and os.path.isdir(fullname)):
					result.append(fullname)
				continue

		# recursively scan other folders, appending results
		if recurse:
			if os.path.isdir(fullname) and not os.path.islink(fullname):
				result = result + Walk( fullname, recurse, pattern, return_folders )

	return result

def arch2xml(root,filename,xsl_file='cfarches.xsl'):
    files = Walk(root, 1, '*.arc', 1)
    print 'searching for arch files in %s' %root
    xml = open(filename,'w')
    xml.write('<?xml version="1.0"?>\n<?xml-stylesheet type="text/xsl" href="%s"?>\n<ARCHES>'%xsl_file)
    for file in files:
            arc = open(file,'r')
            contents = arc.read().split('\n')
            xml.write('<arch>\n')
            mess = 0
            for line in contents:
                    xp = line.split()
                    if mess == 1 and len(xp)>1:
                            str = string.join(xp[0:])
                            xml.write('%s\n' %str)
                    elif len(xp) == 1:
                            tag = string.lower(xp[0])
                            if tag == 'end':
                                    tag = '     <END />\n'
                            elif tag == 'more':
                                    tag = '     <MORE />\n'
                            elif tag =='msg':
                                    tag = '     <message>\n'
                                    mess = 1
                            elif tag =='endmsg':
                                    tag = '     </message>\n'
                                    mess = 0
                            elif tag == 'anim':
                                    tag = '     <anim>\n'
                            elif tag =='mina':
                                    tag = '\n     </anim>\n'
                            else:
                                    tag = '[%s]'%(tag)
                            xml.write('%s' %(tag))
                    elif len(xp)>1:
                            tag = string.lower(xp[0])
                            if (tag[0] == "#"):
                                str = string.join(xp)[1:]
                                xml.write('     <comment>%s</comment>\n' %(str))
                            else:
                                str = string.join(xp[1:])
                                xml.write('     <%s>%s</%s>\n' %(tag,str,tag))
            xml.write('\n</arch>\n')
            arc.close()
    xml.write('\n</ARCHES>')
    xml.close()
    print "DONE"

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 3:
        sys.stderr.write ('Converts arc files in a directory and all sub directories to an xml file\nUsage: arch2xml.py <directory> <XML-filename>\n')
        sys.exit()
    else:
        arch2xml(sys.argv[1],sys.argv[2])
