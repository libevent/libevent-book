
GENERATED_HTML=01_intro.html

all: html examples

html: $(GENERATED_HTML)

examples:
	cd examples_01 && $(MAKE)

01_intro.html: examples_01/01_sync_webclient.c

.txt.html:
	asciidoc $<
clean:
	rm -f *~
	rm -f *.o
	rm -f $(GENERATED_HTML)
	cd examples_01 && $(MAKE) clean