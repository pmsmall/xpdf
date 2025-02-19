//========================================================================
//
// LTKResources.h
//
//========================================================================

#ifndef LTKRESOURCES_H
#define LTKRESOURCES_H

#pragma interface

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <String.h>

extern void ltkGetCmdLineResources(XrmDatabase *cmdLineDB,
				   XrmOptionDescRec *opts,
				   int numOpts, String *appName,
				   int *argc, char *argv[]);

extern void ltkGetOtherResources(Display *display,
				 XrmDatabase cmdLineDB, XrmDatabase *db);

extern String *ltkGetStringResource(XrmDatabase db, String *appName,
				    char *instName, char *def);

extern unsigned long ltkGetColorResource(
                XrmDatabase db, String *appName,
                char *instName,
                Display *display, int screenNum,
                char *def1, unsigned long def2,
                XColor *xcol);

extern XFontStruct *ltkGetFontResouce(XrmDatabase db, String *appName,
				      char *instName,
				      Display *display, int screenNum,
				      char *def);

#endif
