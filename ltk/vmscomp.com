$!========================================================================
$!
$! LTK library compile script for VMS.
$!
$! Copyright 1996 Derek B. Noonburg
$!
$!========================================================================
$!
$! for VAX with gcc
$ CCOMP = "GCC /NOCASE /DEFINE=VMS /INCLUDE=([],[-.GOO])"
$ CXXCOMP = "GCC /PLUSPLUS /NOCASE /DEFINE=VMS /INCLUDE=([],[-.GOO])"
$!
$! for Alpha with DEC compilers
$! CCOMP = "CC /DECC /PREFIX=ALL /DEFINE=VMS /INCLUDE=([],[-.GOO])"
$! CXXCOMP = "CXX /PREFIX=ALL /DEFINE=VMS /INCLUDE=([],[-.GOO])"
$!
$ LTK_OBJS = "LTKApp.obj,LTKBorder.obj,LTKBox.obj,LTKButton.obj," + -
             "LTKCanvas.obj,LTKDblBufCanvas.obj,LTKEmpty.obj," + -
             "LTKLabel.obj,LTKMisc.obj,LTKResources.obj," + -
             "LTKScrollbar.obj,LTKScrollingCanvas.obj,LTKTextIn.obj," + -
             "LTKWidget.obj,LTKWindow.obj"
$!
$ CXXCOMP LTKApp.cc
$ CXXCOMP LTKBorder.cc
$ CXXCOMP LTKBox.cc
$ CXXCOMP LTKButton.cc
$ CXXCOMP LTKCanvas.cc
$ CXXCOMP LTKDblBufCanvas.cc
$ CXXCOMP LTKEmpty.cc
$ CXXCOMP LTKLabel.cc
$ CXXCOMP LTKMisc.cc
$ CXXCOMP LTKResources.cc
$ CXXCOMP LTKScrollbar.cc
$ CXXCOMP LTKScrollingCanvas.cc
$ CXXCOMP LTKTextIn.cc
$ CXXCOMP LTKWidget.cc
$ CXXCOMP LTKWindow.cc
$!
$ lib/cre libltk.olb
$ lib libltk 'LTK_OBJS
$ CXXCOMP LTKbuild.cc
$ link lTKbuild,libltk.olb/lib,[-.goo]libgoo.olb/lib,sys$input/opt
gnu_cc:[000000]gcclib.olb/lib
sys$share:vaxcrtl.exe/share
