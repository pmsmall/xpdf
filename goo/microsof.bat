cl /c /DWIN32 /O2 /Tp gfile.cc gmem.c /Tp gmempp.cc /Tp GString.cc parseargs.c
lib /out:libGoo.lib gfile.obj gmem.obj gmempp.obj GString.obj parseargs.obj
