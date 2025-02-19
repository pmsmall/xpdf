#========================================================================
#
# Xpdf Makefile
#
# Copyright 1996 Derek B. Noonburg
#
#========================================================================

GOODIR = ../goo
LTKDIR = ../ltk

include ../Makefile.config

CXXFLAGS = -O2 -Wall -I$(GOODIR) -I$(LTKDIR) $(CXXFLAGS2)

#------------------------------------------------------------------------

all: xpdf pdftops

#------------------------------------------------------------------------

XPDF_OBJS = Array.o Catalog.o Dict.o Error.o Gfx.o GfxFont.o GfxState.o \
	Lexer.o Link.o Object.o OutputDev.o Page.o Parser.o PSOutput.o \
	Stream.o XOutputDev.o XRef.o xpdf.o
XPDF_LIBS = -L$(LTKDIR) -lLTK -L$(GOODIR) -lGoo $(XLIBS) -lm

xpdf: $(XPDF_OBJS) $(LTKDIR)/libLTK.a $(GOODIR)/libGoo.a
	$(CXX) $(CXXFLAGS) -o xpdf $(XPDF_OBJS) $(XPDF_LIBS)
	strip xpdf

xpdf-ltk.h: xpdf.ltk
	$(LTKDIR)/ltkbuild <xpdf.ltk >xpdf-ltk.h

#------------------------------------------------------------------------

PDFTOPS_OBJS = Array.o Catalog.o Dict.o Error.o Gfx.o GfxFont.o GfxState.o \
	Lexer.o Link.o Object.o OutputDev.o Page.o Parser.o PSOutput.o \
	Stream.o XRef.o pdftops.o
PDFTOPS_LIBS = -L$(GOODIR) -lGoo -lm

pdftops: $(PDFTOPS_OBJS) $(GOODIR)/libGoo.a
	$(CXX) $(CXXFLAGS) -o pdftops $(PDFTOPS_OBJS) $(PDFTOPS_LIBS)
	strip pdftops

#------------------------------------------------------------------------

clean:
	rm -f $(XPDF_OBJS) xpdf-ltk.h xpdf
	rm -f $(PDFTOPS_OBJS) pdftops
	rm -f Makefile.bak

#------------------------------------------------------------------------

depend: xpdf-ltk.h
	cp Makefile Makefile.bak
	sed '/^#----- dependences -----/q' Makefile.bak >Makefile
	echo >>Makefile
	$(CXX) $(CXXFLAGS) -M $(XPDF_OBJS:.o=.cc) pdftops.cc |\
	  sed 's/\/usr\/[^ ]* *//g;/^  \\$$/d' >>Makefile

#----- dependences -----

Array.o: Array.cc \
  ../goo/gmem.h Object.h \
  ../goo/gtypes.h ../goo/GString.h XRef.h Array.h Dict.h \
  Stream.h
Catalog.o: Catalog.cc \
  ../goo/gmem.h Object.h \
  ../goo/gtypes.h ../goo/GString.h XRef.h Array.h Dict.h \
  Stream.h Page.h Error.h Catalog.h
Dict.o: Dict.cc \
  ../goo/gmem.h Object.h \
  ../goo/gtypes.h ../goo/GString.h XRef.h Array.h \
  Dict.h Stream.h
Error.o: Error.cc \
  ../goo/gtypes.h Flags.h \
  Error.h
Gfx.o: Gfx.cc \
  ../goo/gmem.h ../goo/cover.h Object.h ../goo/gtypes.h ../goo/GString.h XRef.h \
  Array.h Dict.h Stream.h Lexer.h Parser.h GfxFont.h GfxState.h OutputDev.h \
  PSOutput.h Flags.h Error.h Gfx.h
GfxFont.o: GfxFont.cc \
  ../goo/GString.h ../goo/gmem.h Object.h \
  ../goo/gtypes.h XRef.h Array.h Dict.h Stream.h Error.h GfxFont.h FontInfo.h
GfxState.o: GfxState.cc \
  ../goo/gmem.h Object.h \
  ../goo/gtypes.h ../goo/GString.h XRef.h Array.h Dict.h Stream.h GfxState.h
Lexer.o: Lexer.cc \
  Lexer.h Object.h \
  ../goo/gtypes.h ../goo/gmem.h ../goo/GString.h XRef.h Array.h Dict.h Stream.h \
  Error.h
Link.o: Link.cc \
  ../goo/gmem.h ../goo/GString.h \
  Error.h \
  Object.h \
  ../goo/gtypes.h XRef.h Array.h Dict.h Stream.h Catalog.h Link.h
Object.o: Object.cc \
  Object.h \
  ../goo/gtypes.h ../goo/gmem.h ../goo/GString.h XRef.h Array.h Dict.h Stream.h \
  Error.h
OutputDev.o: OutputDev.cc \
  OutputDev.h \
  ../goo/gtypes.h
Page.o: Page.cc Object.h \
  ../goo/gtypes.h ../goo/gmem.h ../goo/GString.h XRef.h Array.h Dict.h Stream.h \
  OutputDev.h PSOutput.h Gfx.h Error.h Flags.h Page.h
Parser.o: Parser.cc \
  Object.h \
  ../goo/gtypes.h ../goo/gmem.h ../goo/GString.h XRef.h \
  Array.h Dict.h Stream.h Parser.h Lexer.h Error.h
PSOutput.o: PSOutput.cc \
  ../goo/GString.h \
  config.h Error.h \
  GfxFont.h ../goo/gtypes.h Catalog.h Page.h Object.h \
  ../goo/gmem.h XRef.h Array.h Dict.h Stream.h PSOutput.h
Stream.o: Stream.cc \
  ../goo/gmem.h config.h Error.h Object.h ../goo/gtypes.h \
  ../goo/GString.h XRef.h Array.h Dict.h Stream.h Stream-CCITT.h
XOutputDev.o: XOutputDev.cc \
  ../goo/gmem.h ../goo/GString.h ../ltk/LTKWindow.h \
  ../goo/gtypes.h \
  ../ltk/LTKScrollingCanvas.h ../ltk/LTKWidget.h Object.h XRef.h Array.h Dict.h \
  Stream.h GfxState.h GfxFont.h Error.h Flags.h XOutputDev.h \
  config.h OutputDev.h XOutputFontInfo.h
XRef.o: XRef.cc \
  ../goo/gmem.h Object.h \
  ../goo/gtypes.h ../goo/GString.h XRef.h Array.h Dict.h Stream.h Lexer.h \
  Parser.h Error.h
xpdf.o: xpdf.cc \
  ../goo/parseargs.h \
  ../goo/gtypes.h ../goo/cover.h ../ltk/LTKAll.h ../ltk/LTKApp.h \
  ../goo/GString.h ../ltk/LTKWindow.h \
  ../ltk/LTKWidget.h ../ltk/LTKBox.h ../ltk/LTKBorder.h ../ltk/LTKButton.h \
  ../ltk/LTKCanvas.h ../ltk/LTKDblBufCanvas.h ../ltk/LTKEmpty.h \
  ../ltk/LTKLabel.h ../ltk/LTKScrollbar.h ../ltk/LTKScrollingCanvas.h \
  ../ltk/LTKTextIn.h ../ltk/LTKResources.h Object.h \
  ../goo/gmem.h XRef.h Array.h Dict.h Stream.h Catalog.h Page.h Link.h \
  XOutputDev.h config.h OutputDev.h PSOutput.h Flags.h Error.h leftArrow.xbm \
  rightArrow.xbm zoomIn.xbm zoomOut.xbm rotateCW.xbm rotateCCW.xbm \
  postscript.xbm about.xbm xpdf-ltk.h
pdftops.o: pdftops.cc \
  ../goo/parseargs.h ../goo/gtypes.h ../goo/GString.h \
  Object.h ../goo/gmem.h XRef.h Array.h Dict.h Stream.h Catalog.h Page.h Link.h \
  PSOutput.h Flags.h Error.h config.h
