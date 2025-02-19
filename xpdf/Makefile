include ../Makefile.config

GOODIR = ../goo
LTKDIR = ../ltk

CXXFLAGS = -O -Wall -I$(GOODIR) -L$(GOODIR) -I$(LTKDIR) -L$(LTKDIR) \
	$(CXXFLAGS2)

#------------------------------------------------------------------------

XPDF_OBJS = Array.o Catalog.o Dict.o Error.o Gfx.o GfxFont.o GfxState.o \
	Lexer.o Object.o OutputDev.o Page.o Parser.o \
	Stream.o XOutputDev.o XRef.o xpdf.o
XPDF_LIBS = -lLTK -lGoo $(XLIBS) -lm

xpdf: $(XPDF_OBJS) $(LTKDIR)/libLTK.a $(GOODIR)/libGoo.a
	$(CXX) $(CXXFLAGS) -o xpdf $(XPDF_OBJS) $(XPDF_LIBS)
	strip xpdf

xpdf.ltk.h: xpdf.ltk
	$(LTKDIR)/ltkbuild <xpdf.ltk >xpdf.ltk.h

#------------------------------------------------------------------------

clean:
	rm -f $(XPDF_OBJS) xpdf.ltk.h xpdf
	rm -f Makefile.bak

#------------------------------------------------------------------------

depend: xpdf.ltk.h
	cp Makefile Makefile.bak
	sed '/^#----- dependences -----/q' Makefile.bak >Makefile
	echo >>Makefile
	$(CXX) $(CXXFLAGS) -M $(XPDF_OBJS:.o=.cc) >>Makefile

#----- dependences -----

Array.o: Array.cc ../goo/mem.h Object.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/standards.h /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  ../goo/stypes.h XRef.h Array.h Dict.h Stream.h
Catalog.o: Catalog.cc ../goo/mem.h Object.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/standards.h /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  ../goo/stypes.h XRef.h Array.h Dict.h Stream.h Page.h Error.h Catalog.h
Dict.o: Dict.cc ../goo/mem.h Object.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/standards.h /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  ../goo/stypes.h XRef.h Array.h Dict.h Stream.h
Error.o: Error.cc \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/standards.h /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  ../goo/stypes.h Flags.h Error.h
Gfx.o: Gfx.cc \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/standards.h /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  ../goo/mem.h ../goo/cover.h Object.h ../goo/stypes.h XRef.h Array.h Dict.h \
  Stream.h Lexer.h Parser.h GfxFont.h ../goo/String.h GfxState.h OutputDev.h \
  Flags.h Error.h Gfx.h
GfxFont.o: GfxFont.cc \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdlib.h \
  /usr/include/standards.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/wait.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/resource.h \
  /usr/include/sys/time.h /usr/include/sys/localedef.h \
  /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/include/sys/lc_core.h /usr/include/locale.h \
  /usr/include/sys/localedef31.h ../goo/String.h ../goo/mem.h Object.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  ../goo/stypes.h XRef.h Array.h Dict.h Stream.h Error.h GfxFont.h FontInfo.h
GfxState.o: GfxState.cc \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/math.h \
  /usr/include/standards.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdlib.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/wait.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/resource.h \
  /usr/include/sys/time.h /usr/include/sys/localedef.h \
  /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/include/sys/lc_core.h /usr/include/locale.h \
  /usr/include/sys/localedef31.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  ../goo/mem.h Object.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/va_list.h ../goo/stypes.h XRef.h Array.h Dict.h Stream.h \
  GfxState.h
Lexer.o: Lexer.cc \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdlib.h \
  /usr/include/standards.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/wait.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/resource.h \
  /usr/include/sys/time.h /usr/include/sys/localedef.h \
  /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/include/sys/lc_core.h /usr/include/locale.h \
  /usr/include/sys/localedef31.h /usr/include/ctype.h ../goo/mem.h Lexer.h \
  Object.h /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  ../goo/stypes.h XRef.h Array.h Dict.h Stream.h Error.h
Object.o: Object.cc \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h Object.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/include/standards.h /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  ../goo/stypes.h ../goo/mem.h XRef.h Array.h Dict.h Stream.h Error.h
OutputDev.o: OutputDev.cc OutputDev.h ../goo/stypes.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/standards.h /usr/include/sys/m_types.h
Page.o: Page.cc Object.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/standards.h /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  ../goo/stypes.h ../goo/mem.h XRef.h Array.h Dict.h Stream.h OutputDev.h Gfx.h \
  Error.h Flags.h Page.h
Parser.o: Parser.cc Object.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/standards.h /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  ../goo/stypes.h ../goo/mem.h XRef.h Array.h Dict.h Stream.h Parser.h Lexer.h \
  Error.h
Stream.o: Stream.cc \
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
  /usr/include/sys/mode.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  /usr/include/ctype.h ../goo/mem.h config.h Error.h Object.h ../goo/stypes.h \
  XRef.h Array.h Dict.h Stream.h Stream.CCITT.h
XOutputDev.o: XOutputDev.cc \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/standards.h /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/math.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdlib.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/wait.h \
  /usr/include/sys/resource.h /usr/include/sys/time.h \
  /usr/include/sys/localedef.h /usr/include/sys/lc_core.h \
  /usr/include/locale.h /usr/include/sys/localedef31.h ../ltk/LTKWindow.h \
  /usr/include/X11/Xlib.h /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  ../goo/stypes.h ../goo/String.h ../ltk/LTKScrollingCanvas.h \
  ../ltk/LTKWidget.h ../goo/mem.h Object.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h XRef.h \
  Array.h Dict.h Stream.h GfxState.h GfxFont.h Error.h XOutputDev.h \
  /usr/include/X11/Xutil.h config.h OutputDev.h
XRef.o: XRef.cc \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdlib.h \
  /usr/include/standards.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/wait.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/sys/types.h \
  /usr/include/sys/m_types.h /usr/include/sys/resource.h \
  /usr/include/sys/time.h /usr/include/sys/localedef.h \
  /usr/include/sys/limits.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/float.h \
  /usr/include/sys/lc_core.h /usr/include/locale.h \
  /usr/include/sys/localedef31.h ../goo/mem.h Object.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdio.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stdarg.h \
  /usr/include/va_list.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  ../goo/stypes.h XRef.h Array.h Dict.h Stream.h Lexer.h Parser.h
xpdf.o: xpdf.cc \
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
  /usr/include/X11/cursorfont.h ../goo/parseargs.h ../goo/stypes.h \
  ../goo/cover.h ../ltk/LTKAll.h ../ltk/LTKApp.h /usr/include/X11/Xlib.h \
  /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/X11/Xosdefs.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/stddef.h \
  /usr/include/X11/Xresource.h ../goo/String.h ../ltk/LTKWindow.h \
  ../ltk/LTKWidget.h ../ltk/LTKBox.h ../ltk/LTKBorder.h ../ltk/LTKButton.h \
  ../ltk/LTKCanvas.h ../ltk/LTKDblBufCanvas.h ../ltk/LTKEmpty.h \
  ../ltk/LTKLabel.h ../ltk/LTKScrollbar.h ../ltk/LTKScrollingCanvas.h \
  ../ltk/LTKTextIn.h ../ltk/LTKResources.h Object.h \
  /usr/gnu/lib/gcc-lib/powerpc-ibm-aix4.1.1/2.6.3/include/string.h \
  ../goo/mem.h XRef.h Array.h Dict.h Stream.h Catalog.h Page.h XOutputDev.h \
  /usr/include/X11/Xutil.h config.h OutputDev.h Flags.h Error.h leftArrow.xbm \
  rightArrow.xbm zoomIn.xbm zoomOut.xbm rotateCW.xbm rotateCCW.xbm about.xbm \
  xpdf.ltk.h
