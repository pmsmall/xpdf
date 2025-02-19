#========================================================================
#
# Main xpdf Makefile.
#
# Copyright 1996 Derek B. Noonburg
#
#========================================================================

all:
	cd goo; make depend; make
	cd ltk; make depend; make
	cd xpdf; make depend; make

clean:
	cd goo; make clean
	cd ltk; make clean
	cd xpdf; make clean
