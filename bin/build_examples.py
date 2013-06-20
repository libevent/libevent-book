#!/usr/bin/python
#
# Copyright (c) 2010 Nick Mathewson
# Licensed under the 3-clause BSD license.

"""
   Quick-and-dirty program to pull all the examples out of an asciidoc
   file and make sure we can compile them.

   An example has the format:

   OPTIONAL BUILDLINE
   LABELLINE
   [code,C]
   ------
   (the example code is here)
   ------

   where the LABELLINE starts with .Example or .Bad Example ,
   and where a BUILDLINE (if present) is of the format:

   //BUILD: directives

   Recognized directives are:
     SKIP -- don't build the example
     INC:filename -- add a #include for the filename before the example code
     FUNCTIONBODY -- wrap the example inside a void fn(void) {...} block

   This is not currently meant to be useful for anything but the
   Libevent book.  Right now it only compiles programs, and doesn't try
   to link them.
"""

import os
import re
import subprocess

class FlushFile:
    "replacement-wrapper for stdout that flushes after every write"
    def __init__(self, f):
        self._f = f
    def write(self, s):
        self._f.write(s)
        self._f.flush()

class lookbehind_iterator:
    """Iterator wrapper that remembers the last N items and the total
       item count so far."""
    def __init__(self, iterable, N=1):
        self._iter = iter(iterable)
        self._buf = []
        self._N = N
        self._itemno = 0

    def __iter__(self):
        return self

    def next(self):
        item = self._iter.next() # let StopIteration propagate
        self._itemno += 1
        # This implementation makes iterating through an L-line file
        # into an O(N*L) operation, but so long as N is small, we don't
        # care.
        if len(self._buf) == self._N:
            del(self._buf[0])
        self._buf.append(item)
        return item

    def prev(self, which):
        return self._buf[-(which+1)]

    def getCount(self):
        return self._itemno

def copyUntilEnd(lines, outfile):
    """Copy all of the lines from 'lines' into outfile, until we hit a -----
       line."""
    for line in lines:
        if re.match(r'^\-+$', line):
            return
        outfile.write(line)

def isCode(line):
    """Return true if 'line' is meant to indicate the start of a code section.
       Warn if it does so badly."""
    line = line.strip()
    if line == "[code,C]":
        return True
    elif line.lower().startswith("[code"):
        print "%s should be [code,C]"%line
        return True
    else:
        return False

def isExample(line):
    """Return true if 'line' is meant to label an example code section.
       Warn if we can't tell what it's doing"""
    line = line.strip()
    if line.startswith(".Example") or line.startswith(".Bad Example"):
        return True
    elif line.startswith(".Interface") or line.startswith(".Pseudocode") \
            or line.startswith(".Definition") or \
            line.startswith(".Deprecated Interface"):
        pass
    elif line.startswith("."):
        print "WARN: Weird label %r"%line
    else:
        print "WARN: No label on code section"

    return False

ALL_DIRECTIVES = set([ "SKIP", "FUNCTIONBODY", "INC"])

def getDirectives(line):
    """If 'line' is a build directives line, parse its directives into a
       list of strings (for no-argument directives) and 2-tuples (for
       argument-taking directives)"""
    m = re.match(r'^//\s*BUILD:\s*(.*)', line)
    if not m:
        return ()
    directives_in = m.group(1).split()
    directives = []
    for d in directives_in:
        if ":" in d:
            d,arg = d.split(":", 1)
            directives.append((d.upper(),arg))
        else:
            directives.append(d.upper())

        if d not in ALL_DIRECTIVES:
            print "Unknown directive %r"%d
    return directives

def writepreamble(outfile, directives):
    for d in directives:
        if d[0]=="INC":
            outfile.write('#include "%s"\n'%d[1])

    if "FUNCTIONBODY" in directives:
        outfile.write("void example_func(void) {\n")

def writepostamble(outfile, directives):
    if "FUNCTIONBODY" in directives:
        outfile.write("}\n");

def getCodeBlocks(fname, outputDir):
    """Copy the asciidoc code blocks from an asciidoc file 'fname' into
       separate C files in outputDir.  Return a list of C file names."""

    if not os.path.exists(outputDir):
        os.makedirs(outputDir)

    filelist = []
    counter = 1

    lines = lookbehind_iterator(open(fname, 'r'), 4)
    for line in lines:
        if re.match(r'^\-+$', line):
            prevline = lines.prev(1)
            if isCode(prevline):
                labelline = lines.prev(2)
                directiveline = lines.prev(3)
                directives = getDirectives(directiveline)
                if "SKIP" in directives:
                    continue

                if isExample(labelline):
                    outFname = os.path.join(outputDir, "example%d.c"%counter)
                    counter += 1
                    outfile = open(outFname, 'w')

                    writepreamble(outfile, directives)

                    outfile.write('#line %d "%s"\n'%(lines.getCount()+1, fname))
                    copyUntilEnd(lines, outfile)

                    writepostamble(outfile, directives)

                    outfile.close()
                    filelist.append(outFname)

    return filelist

CC=os.environ.get("CC", "gcc")
CFLAGS=os.environ.get("LEBOOK_CFLAGS", "").split()

def build(fname):
    "Build a C file 'fname'."
    command = [CC, "-Wall", "-c", "-o", "tmp.o" ]
    command.extend(CFLAGS)
    command.append(fname)
    print " ".join(command)
    retcode = subprocess.call(command)

def run(args):
    for fname in args:
        if not "_" in fname:
            print "You broke the naming convention: %s has no _."%fname
            continue
        ident = re.sub(r'([^_]+)_.*', r'\1', fname)
        targetdir = "tmpcode_%s"%ident
        print "Extracting source from %s"%fname
        cfiles = getCodeBlocks(fname, targetdir)
        for cfname in cfiles:
            print "Building %s"%cfname
            build(cfname)

if __name__ == "__main__":
    import sys
    sys.stdout = FlushFile(sys.stdout)
    run(sys.argv[1:])
