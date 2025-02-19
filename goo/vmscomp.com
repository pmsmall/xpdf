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
$ GOO_OBJS = "GString.obj,gmempp.obj,gfile.obj,gmem.obj,parseargs.obj,vms_unlink.obj,vms_directory.obj,vms_unix_times.obj"
$!
$ CXXCOMP GString.cc
$ CXXCOMP gmempp.cc
$ CXXCOMP gfile.cc
$ CCOMP gmem.c
$ CCOMP parseargs.c
$ CCOMP vms_unlink.c
$ CCOMP vms_directory.c
$ CCOMP vms_unix_times.c
$!
$ lib/cre libgoo.olb
$ lib libgoo 'GOO_OBJS
