#========================================================================
#
# LTK library Makefile
#
# Copyright 1996 Derek B. Noonburg
#
#========================================================================

include ../Makefile.config

GOODIR = ../goo

CXXFLAGS = $(CXXFLAGS2) -I$(GOODIR) -I.

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
	$(RANLIB) libLTK.a

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
	$(CXX) $(CXXFLAGS) -M $(LTK_OBJS:.o=.cc) |\
	  sed 's/\/usr\/[^ ]* *//g;/^  \\$$/d' >>Makefile
	$(CXX) $(CXXFLAGS) -M $(LTKBUILD_OBJS:.o=.cc) |\
	  sed 's/\/usr\/[^ ]* *//g;/^  \\$$/d' >>Makefile

#----- dependences -----

LTKApp.o: LTKApp.cc \
 ../goo/gtypes.h LTKApp.h \
 ../goo/GString.h \
 LTKResources.h LTKWindow.h LTKMisc.h LTKWidget.h
LTKBorder.o: LTKBorder.cc \
 ../goo/gtypes.h \
 LTKBorder.h
LTKBox.o: LTKBox.cc \
 LTKWindow.h ../goo/gtypes.h ../goo/GString.h \
 LTKBox.h LTKBorder.h LTKWidget.h
LTKButton.o: LTKButton.cc \
 LTKWindow.h ../goo/gtypes.h ../goo/GString.h \
 LTKButton.h LTKWidget.h LTKBorder.h
LTKCanvas.o: LTKCanvas.cc \
 LTKWindow.h ../goo/gtypes.h ../goo/GString.h \
 LTKCanvas.h LTKWidget.h
LTKDblBufCanvas.o: LTKDblBufCanvas.cc \
 LTKWindow.h ../goo/gtypes.h ../goo/GString.h \
 LTKDblBufCanvas.h LTKWidget.h
LTKEmpty.o: LTKEmpty.cc \
 LTKWindow.h ../goo/gtypes.h ../goo/GString.h \
 LTKEmpty.h LTKWidget.h
LTKLabel.o: LTKLabel.cc \
 LTKLabel.h ../goo/GString.h \
 LTKWidget.h ../goo/gtypes.h LTKWindow.h
LTKMisc.o: LTKMisc.cc \
 LTKMisc.h ../goo/GString.h LTKWindow.h \
 ../goo/gtypes.h LTKWidget.h
LTKResources.o: LTKResources.cc \
 ../goo/gtypes.h ../goo/GString.h \
 LTKConfig.h LTKMisc.h LTKResources.h
LTKScrollbar.o: LTKScrollbar.cc \
 LTKApp.h \
 ../goo/gtypes.h ../goo/GString.h LTKWindow.h \
 LTKScrollbar.h LTKWidget.h LTKBorder.h
LTKScrollingCanvas.o: LTKScrollingCanvas.cc \
 LTKWindow.h ../goo/gtypes.h ../goo/GString.h \
 LTKScrollingCanvas.h LTKWidget.h
LTKTextIn.o: LTKTextIn.cc \
 ../goo/gtypes.h ../goo/GString.h \
 LTKWindow.h LTKTextIn.h LTKWidget.h LTKBorder.h
LTKWidget.o: LTKWidget.cc \
 LTKWindow.h ../goo/gtypes.h ../goo/GString.h \
 LTKWidget.h
LTKWindow.o: LTKWindow.cc \
 LTKConfig.h \
 LTKApp.h ../goo/gtypes.h \
 ../goo/GString.h LTKWindow.h LTKWidget.h LTKBox.h LTKBorder.h
ltkbuild.o: ltkbuild.cc \
 ../goo/gtypes.h \
 ../goo/gmem.h ../goo/GString.h ltkbuild-widgets.h
