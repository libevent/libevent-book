
# asciidoc 8.4.3 and 8.5.2 are known to work.  asciidoc 8.2.5 and 8.2.6 are known not to work.
ASCIIDOC=asciidoc

GENERATED_METAFILES= \
	TOC.html

GENERATED_CHAPTERS= \
	00_about.html \
	01_intro.html \
	Ref0_meta.html \
	Ref1_libsetup.html \
	Ref2_eventbase.html \
	Ref3_eventloop.html \
	Ref4_event.html \
	Ref5_evutil.html \
	Ref6_bufferevent.html \
	Ref6a_advanced_bufferevents.html \
	Ref7_evbuffer.html \
	Ref8_listener.html \
	Ref9_dns.html \
	license_bsd.html

GENERATED_HTML = $(GENERATED_METAFILES) $(GENERATED_CHAPTERS)

all: html examples

# Note that this won't give you good results unless you have a very
# recent asciidoc.  Asciidoc 8.5.3 is recommended.
pdf: LibeventBook.pdf

LibeventBook.pdf: *.txt examples*/*.c
	a2x -f pdf LibeventBook.txt --no-xmllint

html: $(GENERATED_HTML)

check: examples inline_examples

examples:
	cd examples_01 && $(MAKE)
	cd examples_R6 && $(MAKE)
	cd examples_R6a && $(MAKE)
	cd examples_R8 && $(MAKE)
	cd examples_R9 && $(MAKE)

inline_examples:
	./bin/build_examples.py *_*.txt

.SUFFIXES: .txt .html

.txt.html:
	$(ASCIIDOC) $<

00_about.html: license.txt
01_intro.html: examples_01/*.c license.txt
Ref1_meta.html: license.txt
Ref1_libsetup.html: license.txt
Ref2_eventbase.html: license.txt
Ref3_eventloop.html: license.txt
Ref4_event.html: license.txt
Ref5_evutil.html: license.txt
Ref6_bufferevent.html: examples_R6/*.c license.txt
Ref6a_advanced_bufferevent.html: examples_R6a/*.c license.txt
Ref7_evbuffer.html: license.txt
Ref8_listener.html: examples_R8/*.c license.txt
Ref9_dns.html: examples_R9/*.c license.txt

clean:
	rm -f *~
	rm -f *.o
	rm -f $(GENERATED_HTML)
	cd examples_01 && $(MAKE) clean
	cd examples_R8 && $(MAKE) clean
	rm -rf tmpcode_*

count:
	wc -w *.txt
	wc -l examples_*/*.c
