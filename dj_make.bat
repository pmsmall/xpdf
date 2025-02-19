set CC=gcc
set CFLAGS=-g -O2 -DHAVE_DIRENT_H -DHAVE_POPEN -DUSE_GZIP -I..\goo
set CXX=gcc
set CXXFLAGS=%CFLAGS%
set LIBPROG=ar

cd goo
%CXX% %CXXFLAGS% -c GString.cc
%CXX% %CXXFLAGS% -c gmempp.cc
%CXX% %CXXFLAGS% -c gfile.cc
%CC% %CFLAGS% -c gmem.c
%CC% %CFLAGS% -c parseargs.c
del libGoo.a
%LIBPROG% -rc libGoo.a GString.o gmempp.o gfile.o gmem.o parseargs.o

cd ..\xpdf
del *.o
%CXX% %CXXFLAGS% -c Array.cc
%CXX% %CXXFLAGS% -c Catalog.cc
%CXX% %CXXFLAGS% -c Decrypt.cc
%CXX% %CXXFLAGS% -c Dict.cc
%CXX% %CXXFLAGS% -c Error.cc
%CXX% %CXXFLAGS% -c FontEncoding.cc
%CXX% %CXXFLAGS% -c FontFile.cc
%CXX% %CXXFLAGS% -c FormWidget.cc
%CXX% %CXXFLAGS% -c Gfx.cc
%CXX% %CXXFLAGS% -c GfxFont.cc
%CXX% %CXXFLAGS% -c GfxState.cc
%CXX% %CXXFLAGS% -c ImageOutputDev.cc
%CXX% %CXXFLAGS% -c Lexer.cc
%CXX% %CXXFLAGS% -c Link.cc
%CXX% %CXXFLAGS% -c Object.cc
%CXX% %CXXFLAGS% -c OutputDev.cc
%CXX% %CXXFLAGS% -c Page.cc
%CXX% %CXXFLAGS% -c Params.cc
%CXX% %CXXFLAGS% -c Parser.cc
%CXX% %CXXFLAGS% -c PDFDoc.cc
%CXX% %CXXFLAGS% -c PSOutputDev.cc
%CXX% %CXXFLAGS% -c Stream.cc
%CXX% %CXXFLAGS% -c TextOutputDev.cc
%CXX% %CXXFLAGS% -c XRef.cc
del libxpdf.a
%LIBPROG% -rc libxpdf.a *.o

%CXX% %CXXFLAGS% -o pdftops.exe pdftops.cc libxpdf.a ..\goo\libGoo.a

%CXX% %CXXFLAGS% -o pdftotext.exe pdftotext.cc libxpdf.a ..\goo\libGoo.a

%CXX% %CXXFLAGS% -o pdfinfo.exe pdfinfo.cc libxpdf.a ..\goo\libGoo.a

%CXX% %CXXFLAGS% -o pdfimages.exe pdfimages.cc libxpdf.a ..\goo\libGoo.a

cd ..
