#========================================================================
#
# Goo library Makefile
#
# Copyright 1996 Derek B. Noonburg
#
#========================================================================

include ../Makefile.config

CFLAGS = $(CFLAGS2) -I.
CXXFLAGS = $(CXXFLAGS2) -I.

#------------------------------------------------------------------------

GOO_CXX_OBJS = GString.o gmempp.o gfile.o
GOO_C_OBJS = gmem.o parseargs.o cover.o
GOO_OBJS = $(GOO_CXX_OBJS) $(GOO_C_OBJS)

$(LIBPREFIX)Goo.a: $(GOO_OBJS)
	rm -f $(LIBPREFIX)Goo.a
	$(AR) $(LIBPREFIX)Goo.a $(GOO_OBJS)
	$(RANLIB) $(LIBPREFIX)Goo.a

#------------------------------------------------------------------------

clean:
	rm -f $(GOO_OBJS) $(LIBPREFIX)Goo.a
	rm -f Makefile.bak

#------------------------------------------------------------------------

depend:
	cp Makefile Makefile.bak
	sed '/^#----- dependences -----/q' Makefile.bak >Makefile
	echo >>Makefile
	$(CXX) $(CXXFLAGS) -M $(GOO_CXX_OBJS:.o=.cc) |\
	  sed 's/\/usr\/[^ ]* *//g;/^  \\$$/d' |\
	  grep -v '^ \\$$' >>Makefile
	$(CC) $(CFLAGS) -M $(GOO_C_OBJS:.o=.c) |\
	  sed 's/\/usr\/[^ ]* *//g;/^  \\$$/d' |\
	  grep -v '^ \\$$' >>Makefile

#----- dependences -----

GString.o: GString.cc \
 GString.h
gmempp.o: gmempp.cc gmem.h
gfile.o: gfile.cc \
 GString.h gfile.h \
 gtypes.h
gmem.o: gmem.c \
 gmem.h
parseargs.o: parseargs.c \
 parseargs.h gtypes.h
cover.o: cover.c \
 gmem.h cover.h
