#========================================================================
#
# Main xpdf Makefile.
#
# Copyright 1996 Derek B. Noonburg
#
#========================================================================

all:
	cd goo; $(MAKE)
	cd ltk; $(MAKE)
	cd xpdf; $(MAKE) all

xpdf:
	cd goo; $(MAKE)
	cd ltk; $(MAKE)
	cd xpdf; $(MAKE) xpdf

pdftops:
	cd goo; $(MAKE)
	cd xpdf; $(MAKE) pdftops

install:
	install -c xpdf/xpdf $(PREFIX)/bin
	install -c xpdf/pdftops $(PREFIX)/bin

clean:
	cd goo; $(MAKE) clean
	cd ltk; $(MAKE) clean
	cd xpdf; $(MAKE) clean
