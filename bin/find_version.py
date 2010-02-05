#!/usr/bin/python

import re
import sys
import subprocess
import os

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
        if ident.startswith('_'):
            continue

        idmap[ident] = [fname, lineno, False]
    return idmap



def parseVersion(fname):
    m = re.match(r'libevent-(\d+)\.(\d+)(?:\.(\d+)|([a-z]))?', fname)
    assert m
    if m.group(3):
        plev = int(m.group(3))
    elif m.group(4):
        plev = ord(m.group(4)) - ord('a') + 1
    else:
        plev = 0

    return int(m.group(1)), int(m.group(2)), plev

def writeVersion(v):
    v1, v2, v3 = v
    if v3 == 0:
        return "%s.%s" % (v1, v2)
    elif (v1,v2) <= (1,3):
           return "%s.%s%s" % ( v1, v2, chr(ord('a')+v3-1) )
    else:
        return "%s.%s.%s" % ( v1, v2, v3 )

PRIVATE = set("""log.h acconfig.h event-config.h evsignal.h config.h""".split())

def findfiles(direc):
    if os.path.exists(os.path.join(direc, "include", "event2")):
        dirname = os.path.join(direc, "include", "event2")
    else:
        dirname = direc

    contents = [os.path.join(dirname, fn) for fn in os.listdir(dirname)
                if (fn.endswith(".h") and not fn.endswith("-internal.h")
                    and fn not in PRIVATE and not fn.startswith(".")) ]
    return contents

def compile():
    firstSeenIn = {}

    newInVersion = {}

    for d in os.listdir("."):
        if not d.startswith("libevent-"):
            continue
        version = parseVersion(d)
        newInVersion[version] = []
        files = findfiles(d)
        identifiers = getIdentifiers(files)
        for ident in identifiers.iterkeys():
            if firstSeenIn.has_key(ident):
                if firstSeenIn[ident] > version:
                    firstSeenIn[ident] = version
            else:
                firstSeenIn[ident] = version

    for ident,version in sorted(firstSeenIn.items()):
        print ident, writeVersion(version)
        newInVersion[version].append(ident)

    print
    for ver,new in sorted(newInVersion.items()):
        print "==========", writeVersion(ver)
        for ident in new:
            print " ",ident

compile()
