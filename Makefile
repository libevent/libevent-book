
# asciidoc 8.4.3 is known to work.  asciidoc 8.2.5 is known not to work.
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
	Ref7_evbuffer.html \
	Ref8_listener.html

GENERATED_HTML = $(GENERATED_METAFILES) $(GENERATED_CHAPTERS)

all: html examples

html: $(GENERATED_HTML)

check: examples inline_examples

examples:
	cd examples_01 && $(MAKE)
	cd examples_R6 && $(MAKE)
	cd examples_R8 && $(MAKE)

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
Ref6_bufferevent.html: license.txt
Ref7_evbuffer.html: license.txt
Ref8_evbuffer.html: examples_R8/*.c license.txt

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
