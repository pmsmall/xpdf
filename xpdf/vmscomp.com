$!========================================================================
$!
$! Xpdf compile script for VMS.
$!
$! Copyright 1996 Derek B. Noonburg
$!
$!========================================================================
$!
$! for VAX with gcc
$ CXXCOMP = "GCC /PLUSPLUS /NOCASE /DEFINE=(VMS,NO_POPEN,USE_GZIP) /INCLUDE=([],[-.GOO],[-.LTK])"
$! X_LIBS = "SYS$SHARE:DECW$XLIBSHR.EXE/SHARE"
$ open/write optfile VMS.OPT
$ write optfile "GNU_CC:[000000]GCCLIB.OLB/LIB"
$ write optfile "SYS$SHARE:DECW$XLIBSHR.EXE/SHARE"
$ write optfile "SYS$SHARE:VAXCRTL.EXE/SHARE"
$ close optfile 
$!
$! for Alpha with DEC compilers
$! CXXCOMP = "CXX /PREFIX=ALL /DEFINE=(VMS,NO_POPEN,USE_GZIP) /INCLUDE=([],[-.GOO],[-.LTK])"
$! X_LIBS = "SYS$SHARE:DECW$XLIBSHR.EXE/SHARE"
$!
$ XPDF_OBJS = "Array.obj,Catalog.obj,Dict.obj,Error.obj,Gfx.obj," + -
              "GfxFont.obj,GfxState.obj,Lexer.obj,Link.obj,Object.obj," + -
              "OutputDev.obj,Page.obj,Params.obj,Parser.obj,PDFDoc.obj," + -
              "PSOutputDev.obj,Stream.obj,TextOutputDev.obj," + -
              "XOutputDev.obj,XRef.obj"
$ XPDF_LIBS = "[-.goo]libgoo.olb/lib,[-.ltk]libltk.olb/lib"
$!
$ PDFTOPS_OBJS = "Array.obj,Catalog.obj,Dict.obj,Error.obj,Gfx.obj," + -
                 "GfxFont.obj,GfxState.obj,Lexer.obj,Link.obj," + -
                 "Object.obj,OutputDev.obj,Page.obj,Params.obj," + -
                 "Parser.obj,PDFdoc.obj,PSOutputDev.obj,Stream.obj," + -
                 "XRef.obj"
$ PDFTOPS_LIBS = "[-.goo]libgoo.olb/lib"
$!
$ PDFTOTEXT_OBJS = "Array.obj,Catalog.obj,Dict.obj,Error.obj,Gfx.obj," + -
                   "GfxFont.obj,GfxState.obj,Lexer.obj,Link.obj," + -
                   "Object.obj,OutputDev.obj,Page.obj,Params.obj," + -
                   "Parser.obj,PDFdoc.obj,TextOutputDev.obj,Stream.obj," + -
                   "XRef.obj"
$ PDFTOTEXT_LIBS = "[-.goo]libgoo.olb/lib"
$! Build xpdf-ltk.h
$ def/user sys$input xpdf.ltk
$ def/user sys$output xpdf-ltk.h
$ run [-.ltk]ltkbuild
$!
$ CXXCOMP Array.cc
$ CXXCOMP Catalog.cc
$ CXXCOMP Dict.cc
$ CXXCOMP Error.cc
$ CXXCOMP Gfx.cc
$ CXXCOMP GfxFont.cc
$ CXXCOMP GfxState.cc
$ CXXCOMP Lexer.cc
$ CXXCOMP Link.cc
$ CXXCOMP Object.cc
$ CXXCOMP OutputDev.cc
$ CXXCOMP Page.cc
$ CXXCOMP Params.cc
$ CXXCOMP Parser.cc
$ CXXCOMP PDFDoc.cc
$ CXXCOMP PSOutputDev.cc
$ CXXCOMP Stream.cc
$ CXXCOMP TextOutputDev.cc
$ CXXCOMP XOutputDev.cc
$ CXXCOMP XRef.cc
$ CXXCOMP xpdf.cc
$ CXXCOMP pdftops.cc
$ CXXCOMP pdftotext.cc
$!
$ link xpdf,'XPDF_OBJS,'XPDF_LIBS,[]vms.opt/opt
$ link pdftops,'PDFTOPS_OBJS,'PDFTOPS_LIBS,[]vms.opt/opt
$ link pdftotext,'PDFTOTEXT_OBJS,'PDFTOTEXT_LIBS,[]vms.opt/opt
