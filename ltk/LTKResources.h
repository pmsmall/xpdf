//========================================================================
//
// LTKResources.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKRESOURCES_H
#define LTKRESOURCES_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <GString.h>

extern void ltkGetCmdLineResources(XrmDatabase *cmdLineDB,
				   XrmOptionDescRec *opts,
				   int numOpts, GString *appName,
				   int *argc, char *argv[]);

extern void ltkGetOtherResources(Display *display,
				 XrmDatabase cmdLineDB, XrmDatabase *db);

extern GString *ltkGetStringResource(XrmDatabase db, GString *appName,
				     char *instName, char *def);

extern int ltkGetIntResource(XrmDatabase db, GString *appName,
			     char *instName, int def);

extern GBool ltkGetBoolResource(XrmDatabase db, GString *appName,
				char *instName, GBool def);

extern unsigned long ltkGetColorResource(
                XrmDatabase db, GString *appName,
                char *instName,
                Display *display, int screenNum,
                char *def1, unsigned long def2,
                XColor *xcol);

extern XFontStruct *ltkGetFontResouce(XrmDatabase db, GString *appName,
				      char *instName,
				      Display *display, int screenNum,
				      char *def);

extern void ltkGetGeometryResource(XrmDatabase db, GString *appName,
				   char *instName,
				   Display *display, int screenNum,
				   int *x, int *y,
				   Guint *width, Guint *height);

#endif
