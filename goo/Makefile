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

String.o: String.cc \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdlib.h \
  /usr/include/standards.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/wait.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/resource.h \
  /usr/include/sys/time.h /usr/include/sys/localedef.h \
  /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/include/sys/lc_core.h /usr/include/locale.h \
  /usr/include/sys/localedef31.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  ./String.h
mem.o: mem.c \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/standards.h /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdlib.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/wait.h \
  /usr/include/sys/resource.h /usr/include/sys/time.h \
  /usr/include/sys/localedef.h /usr/include/sys/lc_core.h \
  /usr/include/locale.h /usr/include/sys/localedef31.h ./mem.h
parseargs.o: parseargs.c \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/standards.h /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdlib.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/wait.h \
  /usr/include/sys/resource.h /usr/include/sys/time.h \
  /usr/include/sys/localedef.h /usr/include/sys/lc_core.h \
  /usr/include/locale.h /usr/include/sys/localedef31.h /usr/include/ctype.h \
  ./parseargs.h ./stypes.h
cover.o: cover.c \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/standards.h /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h ./mem.h \
  ./cover.h
