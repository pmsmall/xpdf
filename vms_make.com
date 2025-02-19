$!========================================================================
$!
$! Main xpdf compile script for VMS.
$!
$! Copyright 1996 Derek B. Noonburg
$!
$!========================================================================
$!
$! If you want to enable xpm-support for Xpdf invoke this file as 
$! @vms_make xpm . Make sure you've installed both the headerfiles
$! as well as the compiled object library in your X11 path then.
$! Information about the xpm library distribution can be found on 
$! http://axp616.gsi.de:8080/www/vms/sw/xpm.htmlx
$!
$! In case you want to override the automatic compiler detection
$! specify either DECC or GCC as the second paramter,
$! e.g. @vms_make "" GCC 
$!
$! In case of problems with the compile you may contact me at 
$! zinser@decus.decus.de . 
$!
$!========================================================================
$!
$!
$!
$ true = 1
$ false = 0
$ if (p1 .eqs. "") then xpm = ",NO_XPM"
$!
$! Look for the compiler used
$!
$ its_decc = (f$search("SYS$SYSTEM:CXX$COMPILER.EXE") .nes. "")
$ its_gnuc = .not. its_decc  .and. (f$trnlnm("gnu_cc").nes."")
$!
$! Exit if no compiler available
$!
$ if (.not. (its_decc .or. its_gnuc))
$  then
$   write sys$output "C++ compiler required to build Xpdf"
$   exit
$  endif
$!
$! Override if requested from the commandline
$!
$ if (p2 .eqs. "DECC") 
$  then 
$   its_decc = true
$   its_gnuc = false
$ endif
$ if (p2 .eqs. "GCC") 
$  then 
$  its_decc = false
$  its_gnuc = true
$ endif
$!
$ defs = "/define=(VMS,NO_POPEN,USE_GZIP''xpm')"
$ incs = "/include=([],[-.goo],[-.ltk])"
$!
$! Build the option file
$!
$ open/write optf xpdf.opt
$ write optf "Identification=""xpdf 0.90"""
$ if (p1 .eqs. "xpm") then write optf "X11:libxpm.olb/lib"
$ write optf "SYS$SHARE:DECW$XLIBSHR.EXE/SHARE"
$!
$ if its_decc
$  then
$   ccomp   :== "CC /DECC /PREFIX=ALL ''defs' ''incs'"
$   cxxcomp :== "CXX /PREFIX=ALL ''defs' ''incs'"
$ endif
$!
$ if its_gnuc
$  then
$   ccomp   :== "GCC /NOCASE ''defs' ''incs'" 
$   cxxcomp :== "GCC /PLUSPLUS /NOCASE ''defs' ''incs'"
$   write optf "gnu_cc:[000000]gcclib.olb/lib"
$   write optf "sys$share:vaxcrtl.exe/share"
$ endif
$ close optf
$ set default [.goo]
$ @vms_make
$ set default [-.ltk]
$ @vms_make 
$ set default [-.xpdf]
$ @vms_make
$ set default [-]
