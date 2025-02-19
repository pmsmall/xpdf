#========================================================================
#
# Goo library Makefile
#
# Copyright 1996 Derek B. Noonburg
#
#========================================================================

include ../Makefile.config

CFLAGS = -O -Wall -I. $(CFLAGS2)
CXXFLAGS = -O -Wall -I. $(CXXFLAGS2)

#------------------------------------------------------------------------

GOO_CXX_OBJS = GString.o
GOO_C_OBJS = gmem.o parseargs.o cover.o
GOO_OBJS = $(GOO_CXX_OBJS) $(GOO_C_OBJS)

libGoo.a: $(GOO_OBJS)
	rm -f libGoo.a
	ar q libGoo.a $(GOO_OBJS)
	ranlib libGoo.a

#------------------------------------------------------------------------

clean:
	rm -f $(GOO_OBJS) libGoo.a
	rm -f Makefile.bak

#------------------------------------------------------------------------

depend:
	cp Makefile Makefile.bak
	sed '/^#----- dependences -----/q' Makefile.bak >Makefile
	echo >>Makefile
	$(CXX) $(CXXFLAGS) -M $(GOO_CXX_OBJS:.o=.cc) |\
	  sed 's/\/usr\/[^ ]* *//g;/^  \\$$/d' >>Makefile
	$(CC) $(CFLAGS) -M $(GOO_C_OBJS:.o=.c) |\
	  sed 's/\/usr\/[^ ]* *//g;/^  \\$$/d' >>Makefile

#----- dependences -----

GString.o: GString.cc \
  ./GString.h
gmem.o: gmem.c \
  ./gmem.h
parseargs.o: parseargs.c \
  ./parseargs.h ./gtypes.h
cover.o: cover.c \
  ./gmem.h ./cover.h
