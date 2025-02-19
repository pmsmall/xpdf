#========================================================================
#
# LTK library Makefile
#
# Copyright 1996 Derek B. Noonburg
#
#========================================================================

include ../Makefile.config

GOODIR = ../goo

CXXFLAGS = -O -Wall -I$(GOODIR) -I. $(CXXFLAGS2)

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
LTKBUILD_LIBS = -L$(GOODIR) -lGoo

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

LTKApp.o: LTKApp.cc /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/X11/Xlib.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
  /usr/include/X11/Xutil.h ../goo/stypes.h ./LTKApp.h \
  /usr/include/X11/Xresource.h ../goo/String.h ./LTKResources.h ./LTKWindow.h \
  ./LTKMisc.h ./LTKWidget.h
LTKBorder.o: LTKBorder.cc ./LTKBorder.h /usr/include/X11/Xlib.h \
  /usr/include/sys/types.h /usr/include/linux/types.h \
  /usr/include/asm/types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
LTKBox.o: LTKBox.cc /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
  /usr/include/X11/Xutil.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKBox.h ./LTKBorder.h ./LTKWidget.h
LTKButton.o: LTKButton.cc /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
  /usr/include/X11/Xutil.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKButton.h ./LTKWidget.h ./LTKBorder.h
LTKCanvas.o: LTKCanvas.cc /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
  /usr/include/X11/Xutil.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKCanvas.h ./LTKWidget.h
LTKDblBufCanvas.o: LTKDblBufCanvas.cc /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
  /usr/include/X11/Xutil.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKDblBufCanvas.h ./LTKWidget.h
LTKEmpty.o: LTKEmpty.cc /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
  /usr/include/X11/Xutil.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKEmpty.h ./LTKWidget.h
LTKLabel.o: LTKLabel.cc /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
  /usr/include/X11/Xutil.h ./LTKLabel.h ../goo/stypes.h ../goo/String.h \
  ./LTKWidget.h ./LTKWindow.h
LTKMisc.o: LTKMisc.cc /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h \
  /usr/include/stdlib.h /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h \
  /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/unistd.h /usr/include/posix_opt.h /usr/include/gnu/types.h \
  /usr/include/confname.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h /usr/include/pwd.h \
  ./LTKMisc.h ../goo/String.h ./LTKWindow.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/include/X11/Xosdefs.h ../goo/stypes.h ./LTKWidget.h
LTKResources.o: LTKResources.cc /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h \
  ../goo/stypes.h ../goo/String.h ./LTKConfig.h ./LTKMisc.h ./LTKResources.h \
  /usr/include/X11/Xlib.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
  /usr/include/X11/Xresource.h
LTKScrollbar.o: LTKScrollbar.cc /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
  /usr/include/X11/Xutil.h ./LTKApp.h /usr/include/X11/Xresource.h \
  ../goo/stypes.h ../goo/String.h ./LTKWindow.h ./LTKScrollbar.h ./LTKWidget.h \
  ./LTKBorder.h
LTKScrollingCanvas.o: LTKScrollingCanvas.cc /usr/include/stdlib.h \
  /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
  /usr/include/X11/Xutil.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKScrollingCanvas.h ./LTKWidget.h
LTKTextIn.o: LTKTextIn.cc /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
  /usr/include/X11/Xutil.h /usr/include/X11/keysym.h \
  /usr/include/X11/keysymdef.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKTextIn.h ./LTKWidget.h ./LTKBorder.h
LTKWidget.o: LTKWidget.cc /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
  /usr/include/X11/Xutil.h ./LTKWindow.h ../goo/stypes.h ../goo/String.h \
  ./LTKWidget.h
LTKWindow.o: LTKWindow.cc /usr/include/stdlib.h /usr/include/features.h \
  /usr/include/sys/cdefs.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h \
  /usr/include/X11/Xlib.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/X11/X.h \
  /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
  /usr/include/X11/Xutil.h ./LTKConfig.h ./LTKApp.h \
  /usr/include/X11/Xresource.h ../goo/stypes.h ../goo/String.h ./LTKWindow.h \
  ./LTKWidget.h ./LTKBox.h ./LTKBorder.h
ltkbuild.o: ltkbuild.cc /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/libio.h /usr/include/_G_config.h \
  /usr/include/stdlib.h /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h \
  /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/string.h /usr/include/ctype.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h ../goo/stypes.h \
  ../goo/mem.h ../goo/String.h ltkbuild.widgets.h
