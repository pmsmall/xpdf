$!========================================================================
$!
$! Xpdf compile script for VMS.
$!
$! Copyright 1996 Derek B. Noonburg
$!
$!========================================================================
$!
$! for VAX with gcc
$ CCOMP = "GCC /NOCASE /DEFINE=(VMS,NO_POPEN,USE_GZIP) /INCLUDE=([],[-.GOO],[-.LTK])"
$ CXXCOMP = "GCC /PLUSPLUS /NOCASE /DEFINE=(VMS,NO_POPEN,USE_GZIP) /INCLUDE=([],[-.GOO],[-.LTK])"
$! X_LIBS = "SYS$SHARE:DECW$XLIBSHR.EXE/SHARE"
$ open/write optfile VMS.OPT
$ write optfile "GNU_CC:[000000]GCCLIB.OLB/LIB"
$ write optfile "SYS$SHARE:DECW$XLIBSHR.EXE/SHARE"
$ write optfile "SYS$SHARE:VAXCRTL.EXE/SHARE"
$ close optfile 
$!
$! for Alpha with DEC compilers
$! CCOMP = "CC /DECC /PREFIX=ALL /DEFINE=(VMS,NO_POPEN,USE_GZIP) /INCLUDE=([],[-.GOO],[-.LTK])"
$! CXXCOMP = "CXX /PREFIX=ALL /DEFINE=(VMS,NO_POPEN,USE_GZIP) /INCLUDE=([],[-.GOO],[-.LTK])"
$! X_LIBS = "SYS$SHARE:DECW$XLIBSHR.EXE/SHARE"
$!
$ XPDF_OBJS = "Array.obj,Catalog.obj,Dict.obj,Error.obj,Gfx.obj,GfxFont.obj," + -
              "GfxState.obj,Lexer.obj,Link.obj,Object.obj,OutputDev.obj,Page.obj," + -
              "Parser.obj,PSOutput.obj,Stream.obj,XOutputDev.obj,XRef.obj,vms_unlink.obj"
$ XPDF_LIBS = "[-.goo]libgoo.olb/lib,[-.ltk]libltk.olb/lib"
$!
$ PDFTOPS_OBJS = "Array.obj,Catalog.obj,Dict.obj,Error.obj,Gfx.obj,GfxFont.obj," + -
                 "GfxState.obj,Lexer.obj,Link.obj,Object.obj,OutputDev.obj,Page.obj," + -
                 "Parser.obj,PSOutput.obj,Stream.obj,XRef.obj,vms_unlink.obj"
$ PDFTOPS_LIBS = "[-.goo]libgoo.olb/lib"
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
$ CXXCOMP Parser.cc
$ CXXCOMP PSOutput.cc
$ CXXCOMP Stream.cc
$ CXXCOMP XOutputDev.cc
$ CXXCOMP XRef.cc
$ CXXCOMP xpdf.cc
$ CXXCOMP pdftops.cc
$ CCOMP   vms_unlink.c
$!
$ link xpdf,'XPDF_OBJS,'XPDF_LIBS,[]vms.opt/opt
$ link pdftops,'PDFTOPS_OBJS,'PDFTOPS_LIBS,[]vms.opt/opt
