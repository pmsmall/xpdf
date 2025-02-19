#========================================================================
#
# Main xpdf Makefile.
#
# Copyright 1996 Derek B. Noonburg
#
#========================================================================

all:
	cd goo; make
	cd ltk; make
	cd xpdf; make all

xpdf:
	cd goo; make
	cd ltk; make
	cd xpdf; make xpdf

pdftops:
	cd goo; make
	cd xpdf; make pdftops

install:
	install -c xpdf/xpdf $(PREFIX)/bin
	install -c xpdf/pdftops $(PREFIX)/bin

clean:
	cd goo; make clean
	cd ltk; make clean
	cd xpdf; make clean
