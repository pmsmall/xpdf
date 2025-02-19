cl /c /DJAPANESE_SUPPORT /DUSE_GZIP=1 /DWIN32 /I..\goo /O2 /TP @xpdfsrc.txt
lib /out:libxpdf.lib @xpdfobj.txt
cl /DJAPANESE_SUPPORT /DUSE_GZIP=1 /DWIN32 /I..\goo /O2 /Tp pdfimages.cc libxpdf.lib ..\goo\libGoo.lib
cl /DJAPANESE_SUPPORT /DUSE_GZIP=1 /DWIN32 /I..\goo /O2 /Tp pdfinfo.cc libxpdf.lib ..\goo\libGoo.lib
cl /DJAPANESE_SUPPORT /DUSE_GZIP=1 /DWIN32 /I..\goo /O2 /Tp pdftops.cc libxpdf.lib ..\goo\libGoo.lib
cl /DJAPANESE_SUPPORT /DUSE_GZIP=1 /DWIN32 /I..\goo /O2 /Tp pdftotext.cc libxpdf.lib ..\goo\libGoo.lib
