$!========================================================================
$!
$! Goo library compile script for VMS.
$!
$! Copyright 1996 Derek B. Noonburg
$!
$!========================================================================
$!
$! for VAX with gcc
$ CCOMP = "GCC /NOCASE /DEFINE=VMS /INCLUDE=[]"
$ CXXCOMP = "GCC /PLUSPLUS /NOCASE /DEFINE=VMS /INCLUDE=[]"
$!
$! for Alpha with DEC compilers
$! CCOMP = "CC /DECC /PREFIX=ALL /DEFINE=VMS /INCLUDE=[]"
$! CXXCOMP = "CXX /PREFIX=ALL /DEFINE=VMS /INCLUDE=[]"
$!
$ GOO_OBJS = "GString.obj,gmempp.obj,gmem.obj,parseargs.obj,cover.obj"
$!
$ CXXCOMP GString.cc
$ CXXCOMP gmempp.cc
$ CCOMP gmem.c
$ CCOMP parseargs.c
$ CCOMP cover.c
$!
$ lib/cre libgoo.olb
$ lib libgoo 'GOO_OBJS
