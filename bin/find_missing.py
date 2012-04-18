#!/usr/bin/python

import os
import re
import subprocess

WANTED = "defgpstuvx"
#WANTED += "cmn

def getIdentifiers(files):
    command = [ "ctags", "-n", "--c-kinds=+p", "-f", "-" ] + files
    inp = subprocess.Popen(command, stdout=subprocess.PIPE).stdout
    pat = re.compile(r'([^\t]+)\t([^\t]+)\t(\d+);\"\t(.*)')
    idmap = {}
    for line in inp:
        line = line.strip()
        m = pat.match(line)
        assert m
        ident, fname, lineno, ext = m.groups()
        lineno = int(lineno)

        deftype = ext[0]

        if deftype not in WANTED:
            continue
        if (ident.startswith('_EVENT2_') and ident.endswith("_H_") and
            ident == "_EVENT2_%s_H_"%(fname[:-2].upper())):
            continue
        elif ident.startswith("EVENT2_") and ident.endswith("_H_INCLUDED_"):
            continue
        elif ident.startswith('_'):
            continue

        idmap[ident] = [fname, lineno, False]
    return idmap

class CountingIter:
    def __init__(self, iterable):
        self._iter = iter(iterable)
        self._count = 0

    def __iter__(self):
        return self

    def next(self):
        self._count += 1
        return self._iter.next()

    def count(self):
        return self._count

C_IDENT_PAT = re.compile(r'[a-zA-Z_][a-zA-Z0-9_]*')

def removeIdentifiers(idmap, f):
    f = CountingIter(f)
    for line in f:
        if line.startswith("//"):
            # If a function is mentioned in a comment, that doesn't count.
            continue

        if (line.startswith(".Example") or
            line.lower().startswith(".bad example")):
            # If a function is mentioned only in an example blob, that
            # doesn't count either.
            minus_count=0
            for line in f:
                if re.match(r'^\-+', line):
                    minus_count = minus_count + 1
                    if minus_count == 2:
                        break

        words = C_IDENT_PAT.findall(line)
        for w in words:
            if w.endswith("()"):
                w = w[:-2]
            if w in idmap:
                idmap[w][2] = True

def findMissing(includeDir):
    start = os.getcwd()
    try:
        os.chdir(includeDir)
        idmap = getIdentifiers(os.listdir("."))
    finally:
        os.chdir(start)

    for txtFname in sorted(os.listdir(".")):
        if not txtFname.endswith(".txt"):
            continue

        f = open(txtFname, 'r')
        removeIdentifiers(idmap, f)
        f.close()

    byfile = {}
    for ident, (fname, lineno, found) in idmap.iteritems():
        byfile.setdefault(fname, []).append((lineno, ident, found))

    total_n = total_found = 0
    for fname in sorted(byfile.iterkeys()):
        n = len(byfile[fname])
        nfound = sum(1 for tp in byfile[fname] if tp[2])
        print "%s: (%d%%)"%(fname, int(float(100*nfound)/n))

        total_n += n
        total_found += nfound

        for lineno,ident,found in sorted(byfile[fname]):
            if found:
                print "\t    %3d:%s"%(lineno, ident)
            else:
                print "\t*** %3d:%s"%(lineno, ident)

    print "TOTAL: %d%%"%int(float(100*total_found)/total_n)

if __name__ == '__main__':
    import sys
    if len(sys.argv) != 2:
        print \
"""%(prog)s expects a single argument: the location of the installed libevent2
header files.  Example usage: %(prog)s /usr/local/include/event2
It requires a GNU ctags.
""" %{
            "prog":sys.argv[0]
            }
    else:
        findMissing(sys.argv[1])


