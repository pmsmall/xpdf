#========================================================================
#
# Main xpdf Makefile.
#
# Copyright 1996 Derek B. Noonburg
#
#========================================================================

include Makefile.config

all:
	cd goo; $(MAKE)
	cd ltk; $(MAKE)
	cd xpdf; $(MAKE) all

xpdf: dummy
	cd goo; $(MAKE)
	cd ltk; $(MAKE)
	cd xpdf; $(MAKE) xpdf$(EXE)

pdftops: dummy
	cd goo; $(MAKE)
	cd xpdf; $(MAKE) pdftops$(EXE)

pdftotext: dummy
	cd goo; $(MAKE)
	cd xpdf; $(MAKE) pdftotext$(EXE)

install:
	$(INSTALL) xpdf/xpdf$(EXE) $(PREFIX)/bin
	$(INSTALL) xpdf/pdftops$(EXE) $(PREFIX)/bin
	$(INSTALL) xpdf/pdftotext$(EXE) $(PREFIX)/bin
	$(INSTALL) xpdf.1 $(PREFIX)/man/man1
	$(INSTALL) pdftops.1 $(PREFIX)/man/man1
	$(INSTALL) pdftotext.1 $(PREFIX)/man/man1

clean:
	cd goo; $(MAKE) clean
	cd ltk; $(MAKE) clean
	cd xpdf; $(MAKE) clean
dummy:
