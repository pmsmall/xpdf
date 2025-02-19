include ../Makefile.config

GOODIR = ../goo

CXXFLAGS = -O -Wall -I$(GOODIR) -L$(GOODIR) -I. $(CXXFLAGS2)

all: libLTK.a ltkbuild

#------------------------------------------------------------------------

LTK_OBJS =	LTKApp.o \
		LTKBorder.o \
		LTKBox.o \
		LTKButton.o \
		LTKCanvas.o \
		LTKDblBufCanvas.o \
		LTKEmpty.o \
		LTKLabel.o \
		LTKMisc.o \
		LTKResources.o \
		LTKScrollbar.o \
		LTKScrollingCanvas.o \
		LTKTextIn.o \
		LTKWidget.o \
		LTKWindow.o

libLTK.a: $(LTK_OBJS)
	rm -f libLTK.a
	ar q libLTK.a $(LTK_OBJS)
	ranlib libLTK.a

#------------------------------------------------------------------------

LTKBUILD_OBJS = ltkbuild.o
LTKBUILD_LIBS = -lGoo

ltkbuild: $(LTKBUILD_OBJS)
	$(CXX) $(CXXFLAGS) -o ltkbuild $(LTKBUILD_OBJS) $(LTKBUILD_LIBS)

#------------------------------------------------------------------------

clean:
	rm -f $(LTK_OBJS) libLTK.a
	rm -f $(LTKBUILD_OBJS) ltkbuild
	rm -f Makefile.bak

#------------------------------------------------------------------------

depend:
	cp Makefile Makefile.bak
	sed '/^#----- dependences -----/q' Makefile.bak >Makefile
	echo >>Makefile
	$(CXX) $(CXXFLAGS) -M $(LTK_OBJS:.o=.cc) >>Makefile
	$(CXX) $(CXXFLAGS) -M $(LTKBUILD_OBJS:.o=.cc) >>Makefile

#----- dependences -----

LTKApp.o: LTKApp.cc \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdlib.h \
  /usr/include/standards.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/wait.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/resource.h \
  /usr/include/sys/time.h /usr/include/sys/localedef.h \
  /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/include/sys/lc_core.h /usr/include/locale.h \
  /usr/include/sys/localedef31.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  /usr/include/X11/Xutil.h ../goo/stypes.h ./LTKApp.h \
  /usr/include/X11/Xresource.h ../goo/String.h ./LTKResources.h ./LTKWindow.h \
  ./LTKMisc.h ./LTKWidget.h
LTKBorder.o: LTKBorder.cc ./LTKBorder.h /usr/include/X11/Xlib.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/standards.h /usr/include/sys/m_types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h
LTKBox.o: LTKBox.cc \
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
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  /usr/include/X11/Xutil.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKBox.h ./LTKBorder.h ./LTKWidget.h
LTKButton.o: LTKButton.cc \
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
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  /usr/include/X11/Xutil.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKButton.h ./LTKWidget.h ./LTKBorder.h
LTKCanvas.o: LTKCanvas.cc \
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
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  /usr/include/X11/Xutil.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKCanvas.h ./LTKWidget.h
LTKDblBufCanvas.o: LTKDblBufCanvas.cc \
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
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  /usr/include/X11/Xutil.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKDblBufCanvas.h ./LTKWidget.h
LTKEmpty.o: LTKEmpty.cc \
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
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  /usr/include/X11/Xutil.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKEmpty.h ./LTKWidget.h
LTKLabel.o: LTKLabel.cc \
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
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  /usr/include/X11/Xutil.h ./LTKLabel.h ../goo/stypes.h ../goo/String.h \
  ./LTKWidget.h ./LTKWindow.h
LTKMisc.o: LTKMisc.cc \
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
  /usr/include/locale.h /usr/include/sys/localedef31.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/unistd.h \
  /usr/include/sys/access.h /usr/include/sys/lockf.h /usr/include/sys/stat.h \
  /usr/include/sys/mode.h /usr/include/pwd.h ./LTKMisc.h ../goo/String.h \
  ./LTKWindow.h /usr/include/X11/Xlib.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  ../goo/stypes.h ./LTKWidget.h
LTKResources.o: LTKResources.cc \
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
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/va_list.h ../goo/stypes.h ../goo/String.h ./LTKConfig.h \
  ./LTKMisc.h ./LTKResources.h /usr/include/X11/Xlib.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  /usr/include/X11/Xresource.h
LTKScrollbar.o: LTKScrollbar.cc \
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
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  /usr/include/X11/Xutil.h ./LTKApp.h /usr/include/X11/Xresource.h \
  ../goo/stypes.h ../goo/String.h ./LTKWindow.h ./LTKScrollbar.h ./LTKWidget.h \
  ./LTKBorder.h
LTKScrollingCanvas.o: LTKScrollingCanvas.cc \
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
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  /usr/include/X11/Xutil.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKScrollingCanvas.h ./LTKWidget.h
LTKTextIn.o: LTKTextIn.cc \
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
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  /usr/include/X11/Xutil.h /usr/include/X11/keysym.h \
  /usr/include/X11/keysymdef.h /usr/include/X11/aix_keysym.h ./LTKWindow.h \
  ../goo/stypes.h ../goo/String.h ./LTKTextIn.h ./LTKWidget.h ./LTKBorder.h
LTKWidget.o: LTKWidget.cc \
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
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  /usr/include/X11/Xutil.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKWidget.h
LTKWindow.o: LTKWindow.cc \
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
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  /usr/include/X11/Xutil.h ./LTKConfig.h ./LTKApp.h \
  /usr/include/X11/Xresource.h ../goo/stypes.h ../goo/String.h ./LTKWindow.h \
  ./LTKWidget.h ./LTKBox.h ./LTKBorder.h
ltkbuild.o: ltkbuild.cc \
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
  /usr/include/locale.h /usr/include/sys/localedef31.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  /usr/include/ctype.h ../goo/stypes.h ../goo/mem.h ../goo/String.h \
  ltkbuild.widgets.h
