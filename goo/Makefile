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

GOO_CXX_OBJS = String.o
GOO_C_OBJS = mem.o parseargs.o cover.o
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
	$(CXX) $(CXXFLAGS) -M $(GOO_CXX_OBJS:.o=.cc) >>Makefile
	$(CC) $(CFLAGS) -M $(GOO_C_OBJS:.o=.c) >>Makefile

#----- dependences -----

String.o: String.cc /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/string.h ./String.h
mem.o: mem.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h \
  /usr/include/stdlib.h /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h \
  /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  ./mem.h
parseargs.o: parseargs.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h \
  /usr/include/string.h /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h \
  /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/ctype.h ./parseargs.h ./stypes.h
cover.o: cover.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h \
  ./mem.h ./cover.h
