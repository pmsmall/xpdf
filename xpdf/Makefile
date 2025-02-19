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

CXXFLAGS = $(CXXFLAGS2) -I$(GOODIR) -I$(LTKDIR)

#------------------------------------------------------------------------

all: xpdf pdftops

#------------------------------------------------------------------------

XPDF_OBJS = Array.o Catalog.o Dict.o Error.o Gfx.o GfxFont.o \
	GfxState.o Lexer.o Link.o Object.o OutputDev.o Page.o Params.o \
	Parser.o PDFDoc.o PSOutputDev.o Stream.o XOutputDev.o XRef.o \
	xpdf.o
XPDF_LIBS = -L$(LTKDIR) -lLTK -L$(GOODIR) -lGoo $(XLIBS) -lm

xpdf: $(XPDF_OBJS)
	$(CXX) $(CXXFLAGS) -o xpdf $(XPDF_OBJS) $(XPDF_LIBS)
	$(STRIP) xpdf

xpdf-ltk.h: xpdf.ltk
	$(LTKDIR)/ltkbuild <xpdf.ltk >xpdf-ltk.h

#------------------------------------------------------------------------

PDFTOPS_OBJS = Array.o Catalog.o Dict.o Error.o Gfx.o GfxFont.o \
	GfxState.o Lexer.o Link.o Object.o OutputDev.o Page.o Params.o \
	Parser.o PDFDoc.o PSOutputDev.o Stream.o XRef.o pdftops.o
PDFTOPS_LIBS = -L$(GOODIR) -lGoo -lm

pdftops: $(PDFTOPS_OBJS)
	$(CXX) $(CXXFLAGS) -o pdftops $(PDFTOPS_OBJS) $(PDFTOPS_LIBS)
	$(STRIP) pdftops

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
 ../goo/gmem.h \
 Object.h \
 ../goo/gtypes.h ../goo/GString.h Array.h Dict.h Stream.h
Catalog.o: Catalog.cc \
 ../goo/gmem.h \
 Object.h \
 ../goo/gtypes.h ../goo/GString.h Array.h Dict.h \
 Stream.h Page.h Error.h Catalog.h
Dict.o: Dict.cc \
 ../goo/gmem.h \
 Object.h \
 ../goo/gtypes.h ../goo/GString.h Array.h Dict.h Stream.h XRef.h
Error.o: Error.cc \
 ../goo/gtypes.h \
 Params.h Error.h
Gfx.o: Gfx.cc \
 ../goo/gmem.h ../goo/cover.h Object.h \
 ../goo/gtypes.h ../goo/GString.h Array.h Dict.h Stream.h Lexer.h \
 Parser.h GfxFont.h GfxState.h OutputDev.h Params.h Error.h Gfx.h
GfxFont.o: GfxFont.cc \
 ../goo/GString.h ../goo/gmem.h ../goo/fileNames.h ../goo/gtypes.h \
 Object.h Array.h Dict.h Stream.h Error.h Params.h GfxFont.h \
 FontInfo.h
GfxState.o: GfxState.cc \
 ../goo/gmem.h \
 Object.h \
 ../goo/gtypes.h ../goo/GString.h Array.h Dict.h Stream.h GfxState.h
Lexer.o: Lexer.cc \
 Lexer.h Object.h \
 ../goo/gtypes.h ../goo/gmem.h \
 ../goo/GString.h Array.h Dict.h Stream.h Error.h
Link.o: Link.cc \
 ../goo/gmem.h \
 ../goo/GString.h \
 Error.h Object.h ../goo/gtypes.h Array.h Dict.h Stream.h Link.h
Object.o: Object.cc \
 Object.h \
 ../goo/gtypes.h \
 ../goo/gmem.h ../goo/GString.h Array.h Dict.h Stream.h Error.h XRef.h
OutputDev.o: OutputDev.cc \
 OutputDev.h \
 ../goo/gtypes.h
Page.o: Page.cc \
 Object.h \
 ../goo/gtypes.h \
 ../goo/gmem.h ../goo/GString.h Array.h Dict.h Stream.h XRef.h \
 OutputDev.h Gfx.h Error.h Params.h Page.h
Params.o: Params.cc \
 ../goo/gtypes.h ../goo/gmem.h ../goo/GString.h ../goo/fileNames.h \
 Params.h
Parser.o: Parser.cc \
 Object.h \
 ../goo/gtypes.h \
 ../goo/gmem.h ../goo/GString.h Array.h Dict.h Stream.h Parser.h \
 Lexer.h Error.h
PDFDoc.o: PDFDoc.cc \
 ../goo/GString.h Page.h Object.h ../goo/gtypes.h ../goo/gmem.h \
 Array.h Dict.h Stream.h Catalog.h XRef.h Link.h OutputDev.h Params.h \
 Error.h PDFDoc.h
PSOutputDev.o: PSOutputDev.cc \
 ../goo/GString.h \
 config.h Object.h ../goo/gtypes.h ../goo/gmem.h Array.h Dict.h \
 Stream.h Error.h GfxState.h GfxFont.h Catalog.h Page.h PSOutputDev.h \
 OutputDev.h
Stream.o: Stream.cc \
 ../goo/gmem.h config.h \
 Error.h Object.h ../goo/gtypes.h ../goo/GString.h Array.h Dict.h \
 Stream.h Stream-CCITT.h
XOutputDev.o: XOutputDev.cc \
 ../goo/gmem.h \
 ../goo/GString.h ../ltk/LTKWindow.h \
 ../goo/gtypes.h \
 ../ltk/LTKScrollingCanvas.h ../ltk/LTKWidget.h Object.h Array.h \
 Dict.h Stream.h GfxState.h GfxFont.h Error.h Params.h XOutputDev.h \
 config.h OutputDev.h XOutputFontInfo.h
XRef.o: XRef.cc \
 ../goo/gmem.h \
 Object.h ../goo/gtypes.h ../goo/GString.h \
 Array.h Dict.h Stream.h Lexer.h Parser.h Error.h XRef.h
xpdf.o: xpdf.cc \
 ../goo/gtypes.h ../goo/GString.h ../goo/parseargs.h \
 ../goo/fileNames.h ../goo/cover.h ../ltk/LTKAll.h ../ltk/LTKApp.h \
 ../ltk/LTKWindow.h ../ltk/LTKWidget.h ../ltk/LTKBox.h \
 ../ltk/LTKBorder.h ../ltk/LTKButton.h ../ltk/LTKCanvas.h \
 ../ltk/LTKDblBufCanvas.h ../ltk/LTKEmpty.h ../ltk/LTKLabel.h \
 ../ltk/LTKScrollbar.h ../ltk/LTKScrollingCanvas.h ../ltk/LTKTextIn.h \
 ../ltk/LTKResources.h Object.h \
 ../goo/gmem.h Array.h Dict.h Stream.h XRef.h Catalog.h Page.h Link.h \
 PDFDoc.h XOutputDev.h config.h OutputDev.h PSOutputDev.h Params.h \
 Error.h leftArrow.xbm dblLeftArrow.xbm rightArrow.xbm \
 dblRightArrow.xbm zoomIn.xbm zoomOut.xbm rotateCW.xbm rotateCCW.xbm \
 postscript.xbm about.xbm xpdf-ltk.h
pdftops.o: pdftops.cc \
 ../goo/parseargs.h ../goo/gtypes.h ../goo/GString.h Object.h \
 ../goo/gmem.h Array.h Dict.h Stream.h XRef.h Catalog.h Page.h \
 PDFDoc.h Link.h PSOutputDev.h config.h OutputDev.h Params.h Error.h
