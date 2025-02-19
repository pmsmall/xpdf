set CC=cl
set CFLAGS=/DJAPANESE_SUPPORT#1 /DUSE_GZIP#1 /DWIN32 /I..\goo /O2 /nologo
set CXX=cl
set CXXFLAGS=%CFLAGS% /TP
set LIBPROG=lib

cd goo
%CXX% %CXXFLAGS% /c GString.cc
%CXX% %CXXFLAGS% /c gmempp.cc
%CXX% %CXXFLAGS% /c gfile.cc
%CC% %CFLAGS% /c gmem.c
%CC% %CFLAGS% /c parseargs.c
%LIBPROG% /nologo /out:libGoo.lib GString.obj gmempp.obj gfile.obj gmem.obj parseargs.obj

cd ..\xpdf
%CXX% %CXXFLAGS% /c Array.cc
%CXX% %CXXFLAGS% /c Catalog.cc
%CXX% %CXXFLAGS% /c Decrypt.cc
%CXX% %CXXFLAGS% /c Dict.cc
%CXX% %CXXFLAGS% /c Error.cc
%CXX% %CXXFLAGS% /c FontEncoding.cc
%CXX% %CXXFLAGS% /c FontFile.cc
%CXX% %CXXFLAGS% /c FormWidget.cc
%CXX% %CXXFLAGS% /c Function.cc
%CXX% %CXXFLAGS% /c Gfx.cc
%CXX% %CXXFLAGS% /c GfxFont.cc
%CXX% %CXXFLAGS% /c GfxState.cc
%CXX% %CXXFLAGS% /c ImageOutputDev.cc
%CXX% %CXXFLAGS% /c Lexer.cc
%CXX% %CXXFLAGS% /c Link.cc
%CXX% %CXXFLAGS% /c Object.cc
%CXX% %CXXFLAGS% /c OutputDev.cc
%CXX% %CXXFLAGS% /c Page.cc
%CXX% %CXXFLAGS% /c Params.cc
%CXX% %CXXFLAGS% /c Parser.cc
%CXX% %CXXFLAGS% /c PDFDoc.cc
%CXX% %CXXFLAGS% /c PSOutputDev.cc
%CXX% %CXXFLAGS% /c Stream.cc
%CXX% %CXXFLAGS% /c TextOutputDev.cc
%CXX% %CXXFLAGS% /c XRef.cc
%CXX% %CXXFLAGS% /c pdftops.cc
%CXX% %CXXFLAGS% /c pdftotext.cc
%CXX% %CXXFLAGS% /c pdfinfo.cc
%CXX% %CXXFLAGS% /c pdfimages.cc

%CXX% /nologo /Fepdftops.exe Array.obj Catalog.obj Decrypt.obj Dict.obj Error.obj FontEncoding.obj FontFile.obj FormWidget.obj Function.obj Gfx.obj GfxFont.obj GfxState.obj Lexer.obj Link.obj Object.obj OutputDev.obj Page.obj Params.obj Parser.obj PDFDoc.obj PSOutputDev.obj Stream.obj XRef.obj pdftops.obj ..\goo\libGoo.lib

%CXX% /nologo /Fepdftotext.exe Array.obj Catalog.obj Decrypt.obj Dict.obj Error.obj FontEncoding.obj FontFile.obj FormWidget.obj Function.obj Gfx.obj GfxFont.obj GfxState.obj Lexer.obj Link.obj Object.obj OutputDev.obj Page.obj Params.obj Parser.obj PDFDoc.obj TextOutputDev.obj Stream.obj XRef.obj pdftotext.obj ..\goo\libGoo.lib

%CXX% /nologo /Fepdfinfo.exe Array.obj Catalog.obj Decrypt.obj Dict.obj Error.obj FontEncoding.obj FontFile.obj FormWidget.obj Function.obj Gfx.obj GfxFont.obj GfxState.obj Lexer.obj Link.obj Object.obj OutputDev.obj Page.obj Params.obj Parser.obj PDFDoc.obj Stream.obj XRef.obj pdfinfo.obj ..\goo\libGoo.lib

%CXX% /nologo /Fepdfimages.exe Array.obj Catalog.obj Decrypt.obj Dict.obj Error.obj FontEncoding.obj FontFile.obj FormWidget.obj Function.obj Gfx.obj GfxFont.obj GfxState.obj ImageOutputDev.obj Lexer.obj Link.obj Object.obj OutputDev.obj Page.obj Params.obj Parser.obj PDFDoc.obj Stream.obj XRef.obj pdfimages.obj ..\goo\libGoo.lib

cd ..
