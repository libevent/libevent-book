
# asciidoc 8.4.3 is known to work.  asciidoc 8.2.5 is known not to work.
ASCIIDOC=asciidoc

GENERATED_HTML= \
	TOC.html \
	00_about.html \
	01_intro.html \
	Ref1_libsetup.html \
	Ref2_eventbase.html \
	Ref3_eventloop.html \
	Ref4_event.html \
	Ref5_evutil.html

all: html examples

html: $(GENERATED_HTML)

examples:
	cd examples_01 && $(MAKE)

.SUFFIXES: .txt .html

.txt.html:
	$(ASCIIDOC) $<

00_about.html: license.txt
01_intro.html: examples_01/*.c license.txt
Ref1_libsetup.html: license.txt
Ref2_eventbase.html: license.txt
Ref3_eventloop.html: license.txt
Ref4_event.html: license.txt
Ref5_evutil.html: license.txt

clean:
	rm -f *~
	rm -f *.o
	rm -f $(GENERATED_HTML)
	cd examples_01 && $(MAKE) clean

count:
	wc -w *.txt
	wc -l examples_*/*.c
