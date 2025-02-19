$!========================================================================
$!
$! LTK library compile script for VMS.
$!
$! Written by Patrick Moreau, Martin P.J. Zinser.
$!
$! Copyright 1996 Derek B. Noonburg
$!
$!========================================================================
$!
$!
$ LTK_OBJS = "LTKApp.obj,LTKBorder.obj,LTKBox.obj,LTKButton.obj," + -
             "LTKButtonDialog.obj,LTKCanvas.obj,LTKCompoundWidget.obj," + -
             "LTKDblBufCanvas.obj,LTKEmpty.obj,LTKFileReq.obj," + -
             "LTKLabel.obj,LTKList.obj,LTKMenu.obj,LTKMenuButton.obj," + -
             "LTKMisc.obj,LTKResources.obj,LTKScrollbar.obj," + -
             "LTKScrollingCanvas.obj,LTKTextIn.obj,LTKWidget.obj," + -
             "LTKWindow.obj"
$!
$ i = 0
$COMPILE_CXX_LOOP:
$ file = f$element(i, ",",LTK_OBJS)
$ if file .eqs. "," then goto COMPILE_END
$ i = i + 1
$ name = f$parse(file,,,"NAME")
$ call make 'file "CXXCOMP ''name'.cc" - 
       'name'.cc
$ goto COMPILE_CXX_LOOP
$!
$COMPILE_END:
$ call make libltk.olb "lib/cre libltk.olb *.obj" *.obj
$ call make LTKbuild.obj "CXXCOMP LTKbuild.cc" - 
       LTKbuild.cc
$ call make LTKbuild.exe - 
    "xpdf_link lTKbuild,libltk.olb/lib,[-.goo]libgoo.olb/lib,[-]xpdf.opt/opt" -
    LTKbuild.obj libltk.olb [-.goo]libgoo.olb
$!
$ exit
$!
$MAKE: SUBROUTINE   !SUBROUTINE TO CHECK DEPENDENCIES
$ V = 'F$Verify(0)
$! P1 = What we are trying to make
$! P2 = Command to make it
$! P3 - P8  What it depends on
$
$ If F$Search(P1) .Eqs. "" Then Goto Makeit
$ Time = F$CvTime(F$File(P1,"RDT"))
$arg=3
$Loop:
$       Argument = P'arg
$       If Argument .Eqs. "" Then Goto Exit
$       El=0
$Loop2:
$       File = F$Element(El," ",Argument)
$       If File .Eqs. " " Then Goto Endl
$       AFile = ""
$Loop3:
$       OFile = AFile
$       AFile = F$Search(File)
$       If AFile .Eqs. "" .Or. AFile .Eqs. OFile Then Goto NextEl
$       If F$CvTime(F$File(AFile,"RDT")) .Ges. Time Then Goto Makeit
$       Goto Loop3
$NextEL:
$       El = El + 1
$       Goto Loop2
$EndL:
$ arg=arg+1
$ If arg .Le. 8 Then Goto Loop
$ Goto Exit
$
$Makeit:
$ VV=F$VERIFY(0)
$ write sys$output P2
$ 'P2
$ VV='F$Verify(VV)
$Exit:
$ If V Then Set Verify
$ENDSUBROUTINE
