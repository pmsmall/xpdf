$!========================================================================
$!
$! Main xpdf compile script for VMS.
$!
$! Written by Patrick Moreau, Martin P.J. Zinser.
$!
$! Copyright 1996 Derek B. Noonburg
$!
$!========================================================================
$!
$! This procedure takes up to three parameters:
$!
$! 1.) Configuration settings:
$!
$!     xpm - Enable XPM support for Xpdf.Make sure you've installed both 
$!     the headerfiles as well as the compiled object library in your X11 
$!     path then. Information about the xpm library distribution can be 
$!     found on http://www.decus.de:8080/www/vms/sw/xpm.htmlx
$!
$!     a4 - Use european A4 as the default paper size.
$!
$!     japanese - Compile Xpdf with support for Japanese fonts
$!
$!     compress - Use compress to decode embeded images (instead of the 
$!     default gzip)
$!
$! 2.) Compiler detection:
$!
$!     In case you want to override the automatic compiler detection
$!     specify either DECC or GCC as the second paramter,
$!     e.g. @vms_make "" GCC 
$!
$! 3.) T1lib support
$!
$!     In case you want to build a copy of xpdf with t1lib you need to tell
$!     the script where to find the header files and object library. The format
$!     of this parameter is (inc=<dir>,olb=<lib>). For use with Xpdf on OpenVMS
$!     t1lib 1.0.0 plus a patch is required. For further information please
$!     check http://www.decus.de:8080/www/vms/sw/t1lib.htmlx
$!
$! 4.) Freetype support
$!
$!     Use Freetype 1.3.1 from
$!     http://download.sourceforge.net/freetype/freetype-1.3.1.tar.gz
$!     which builds out of the box on VMS with MMS
$!
$! Sample invocation of the script:
$!    @vms_make xpm,a4,japanese "" -
$!             (inc=public$root:[xtools.libs.t1lib.lib.t1lib],olb=pubbin:t1.olb)
$!
$! In case of problems with the compile you may contact me at 
$! zinser@decus.de (preferred) or zinser@sysdev.exchange.de (work). 
$!
$!========================================================================
$!
$!
$! Just some general constants...
$!
$ true  = 1
$ false = 0
$ xpdf_link :== link
$!
$! Setup variables holding "config" information
$!
$ mydefs       = ""
$ myincs       = ""
$ cxxdefs      = ""
$ t1inc        = ""
$ t1olb        = ""
$ freeinc      = ""
$ freeinc2     = ""
$ freeolb      = ""
$ float        = ""
$ xpm_def      = false
$ a4_def       = false
$ japanese_def = false
$ compress_def = false
$!
$! Process "config" settings (analog to Unix --enable-xxx)
$!
$ if (p1.nes."")
$ then
$   i = 0
$   qual_list = f$edit(p1,"upcase")
$DEF_LOOP:
$   qual = f$element(i,",",qual_list)
$   if qual.eqs."," then goto FINISH_DEF
$   i = i + 1
$   if (qual .eqs. "XPM")
$   then
$     xpm_def = true
$     goto def_loop
$   endif
$   if (qual .eqs. "A4")
$   then
$     mydefs = mydefs + ",A4_PAPER"
$     a4_def = true
$     goto def_loop
$   endif
$   if (qual .eqs. "JAPANESE")
$   then
$     mydefs = mydefs + ",JAPANESE_SUPPORT=1"
$     japanese_def = true
$     goto def_loop
$   endif
$   if (qual .eqs. "COMPRESS")
$   then
$     compress_def = true
$     goto def_loop
$   endif
$   write sys$output "Qualifier ''qual' no recognized, will be ignored"
$   goto def_loop
$ endif
$FINISH_DEF:
$ if .not.xpm_def then mydefs = mydefs + ",NO_XPM"
$ if .not.compress_def then mydefs = mydefs + ",USE_GZIP"
$!
$! Check if we should build an xpdf with support for t1lib
$!
$ if (p3.nes."")
$ then
$   i = 0
$   t1qual = f$edit(p3,"upcase")
$   start = f$locate("(",t1qual) + 1
$   len = f$length(t1qual) - start - 1
$   t1qual = f$extract(start,len,t1qual)
$T1_LOOP:
$   t1val = f$element(i,",",t1qual)
$   if t1val.eqs."," then goto FINISH_T1
$   i = i + 1
$   start = f$locate("=",t1val) + 1
$   len = f$length(t1val) - start
$   dir = f$extract(start,len,t1val) 
$   if (f$extract(0,3,t1val).eqs."INC")
$   then
$     if f$search("''dir't1lib.h").nes.""
$     then
$       t1inc = dir
$     else
$       write sys$output "Canot find t1lib.h in directory ''dir' - Exiting"
$       exit
$     endif
$     goto t1_loop
$   endif       
$   if (f$extract(0,3,t1val).eqs."OLB")
$   then
$     if f$search("''dir'").nes.""
$     then
$       t1olb = dir
$     else
$       write sys$output "Canot find t1 object library ''dir' - Exiting"
$       exit
$     endif
$     goto t1_loop
$   endif       
$ endif
$FINISH_T1:
$ if (t1inc.nes."").and.(t1olb.nes."")
$ then
$   mydefs = mydefs + ",HAVE_T1LIB_H=1,HAVE_T1LIBX_H=1"
$   myincs = myincs + "," + t1inc
$   if f$getsyi("HW_MODEL").ge.1024 then float  = "/float=ieee_float"
$ endif
$!
$! Freetype  (for testing)
$!
$ if (freeinc.nes."").and(freeinc2.nes."").and.(freeolb.nes."")
$ then
$   mydefs = mydefs + ",HAVE_FREETYPE_H"
$   myincs = myincs + "," + freeinc + "," + freeinc2
$ endif
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
$ defs = "/define=(VMS,NO_POPEN''mydefs')"
$ incs = "/include=([],[-.goo],[-.ltk]''myincs')"
$!
$! Build the option file
$!
$ open/write optf xpdf.opt
$ write optf "Identification=""xpdf 0.91"""
$ if (xpm_def) then write optf "X11:libxpm.olb/lib"
$ if (t1inc.nes."").and.(t1olb.nes."") then write optf t1olb, "/lib"
$! freetype
$ if (freeinc.nes."").and.(freeinc2.nes."").and.(freeolb.nes."") then write optf freeolb, "/lib"
$ write optf "SYS$SHARE:DECW$XLIBSHR.EXE/SHARE"
$!
$ if its_decc
$  then
$   ccomp   :== "CC /DECC /PREFIX=ALL ''float' ''defs' ''incs'"
$!
$!  Check version of the C++ compiler
$!
$   create vms_xpdf_cc_test.cc
$   cxx/lis=vms_xpdf_cc_test.lis/show=all vms_xpdf_cc_test.cc
$   open list vms_xpdf_cc_test.lis
$CXX_LIST:
$   read/end=close_cxx list line
$   start = f$locate("__DECCXX_VER",line)
$   if start.ne.f$length(line)
$   then
$     cxx_ver = f$extract(start+13,8,line)
$     if cxx_ver .gt. 60000000 
$     then 
$       cxxdefs = "/warn=(disable=nosimpint)"
$       xpdf_link :== cxxlink
$     endif
$     goto close_cxx
$   endif
$   goto cxx_list
$CLOSE_CXX: 
$   close list 
$   delete/noconfirm vms_xpdf_cc_test.*;*
$   cxxcomp :== "CXX /PREFIX=ALL ''cxxdefs' ''float' ''defs' ''incs'"
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
$ write sys$output "Compiling in [.GOO]"
$ set default [.goo]
$ @vms_make
$ write sys$output "Compiling in [.LTK]"
$ set default [-.ltk]
$ @vms_make 
$ write sys$output "Compiling in [.XPDF]"
$ set default [-.xpdf]
$ @vms_make
$ set default [-]
$ delete/sym/glob cxxcomp
$ delete/sym/glob ccomp
$ delete/sym/glob xpdf_link
$ exit
