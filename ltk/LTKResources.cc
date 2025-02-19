//========================================================================
//
// LTKResources.cc
//
// Copyright 1996-2002 Glyph & Cog, LLC
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <aconf.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include "gtypes.h"
#include "GString.h"
#include "gfile.h"
#include "LTKConfig.h"
#include "LTKMisc.h"
#include "LTKResources.h"

// this can be set from the configure script
#ifndef APPDEFDIR
#define APPDEFDIR "/usr/lib/X11/app-defaults"
#endif

void ltkGetCmdLineResources(XrmDatabase *cmdLineDB, XrmOptionDescRec *opts,
			    int numOpts, GString *appName,
			    int *argc, char *argv[]) {
  XrmParseCommand(cmdLineDB, opts, numOpts, appName->getCString(),
		  argc, argv);
}

void ltkGetOtherResources(Display *display, GString *appClass,
			  XrmDatabase cmdLineDB, XrmDatabase *db) {
  XrmDatabase db1;
  GString *s;

  // application defaults
  s = appendToPath(new GString(APPDEFDIR), appClass->getCString());
  if ((db1 = XrmGetFileDatabase(s->getCString()))) {
    XrmMergeDatabases(db1, db);
  }
  delete s;

  // user resources
  if (XResourceManagerString(display)) {
    db1 = XrmGetStringDatabase(XResourceManagerString(display));
    XrmMergeDatabases(db1, db);
  } else {
#ifdef VMS
    s = new GString("DECW$USER_DEFAULTS:XPDF.DAT");
#else
    s = appendToPath(getHomeDir(), ".Xresources");
#endif
    if (!(db1 = XrmGetFileDatabase(s->getCString()))) {
      XrmMergeDatabases(db1, db);
    }
    delete s;
#ifdef VMS
    s = new GString("DECW$USER_DEFAULTS:DECW$XDEFAULTS.DAT");
#else
    s = appendToPath(getHomeDir(), ".Xdefaults");
#endif
    if ((db1 = XrmGetFileDatabase(s->getCString()))) {
      XrmMergeDatabases(db1, db);
    }
    delete s;
  }

  // command line options
  XrmMergeDatabases(cmdLineDB, db);
}

GString *ltkGetStringResource(XrmDatabase db,
			      GString *appName, GString *appClass,
			      char *instName, char *def) {
  GString *inst = appName->copy()->append(".")->append(instName);
  GString *clas = appClass->copy()->append(".")->append(instName);
  XrmValue val;
  char *resType[20];
  GString *ret;

  if (XrmGetResource(db, inst->getCString(), clas->getCString(),
		     resType, &val))
    ret = new GString(val.addr, val.size);
  else
    ret = def ? new GString(def) : (GString *)NULL;
  delete inst;
  delete clas;
  return ret;
}

int ltkGetIntResource(XrmDatabase db,
		      GString *appName, GString *appClass,
		      char *instName, int def) {
  GString *inst = appName->copy()->append(".")->append(instName);
  GString *clas = appClass->copy()->append(".")->append(instName);
  XrmValue val;
  char *resType[20];
  char s[20];
  int ret, n;

  if (XrmGetResource(db, inst->getCString(), clas->getCString(),
		     resType, &val)) {
    n = val.size <= 19 ? val.size : 19;
    strncpy(s, val.addr, n);
    s[n] = '\0';
    ret = atoi(s);
  } else {
    ret = def;
  }
  delete inst;
  delete clas;
  return ret;
}

GBool ltkGetBoolResource(XrmDatabase db,
			 GString *appName, GString *appClass,
			 char *instName, GBool def) {
  GString *inst = appName->copy()->append(".")->append(instName);
  GString *clas = appClass->copy()->append(".")->append(instName);
  XrmValue val;
  char *resType[20];
  char *s;
  GBool ret;

  if (XrmGetResource(db, inst->getCString(), clas->getCString(),
		     resType, &val)) {
    s = (char *)val.addr;
    ret =  ((s[0] == 'o' || s[0] == 'O') && (s[1] == 'n' || s[1] == 'N')) ||
	   ((s[0] == 't' || s[0] == 'T') && (s[1] == 'r' || s[1] == 'R') &&
	    (s[2] == 'u' || s[2] == 'U') && (s[3] == 'e' || s[3] == 'E')) ||
	   ((s[0] == 'y' || s[0] == 'Y') && (s[1] == 'e' || s[1] == 'E') &&
	    (s[2] == 's' || s[2] == 'S'));
  } else {
    ret = def;
  }
  delete inst;
  delete clas;
  return ret;
}

unsigned long ltkGetColorResource(XrmDatabase db,
				  GString *appName, GString *appClass,
				  char *instName,
				  Display *display, int screenNum,
				  char *def1, unsigned long def2,
				  XColor *xcol) {
  GString *name;
  XColor xcol1, xcol2;

  if (!xcol)
    xcol = &xcol1;
  name = ltkGetStringResource(db, appName, appClass, instName, def1);
  if (!XAllocNamedColor(display, DefaultColormap(display, screenNum),
			name->getCString(), xcol, &xcol2)) {
    ltkError("Couldn't allocate color '%s'", name->getCString());
    xcol->pixel = def2;
    XQueryColor(display, DefaultColormap(display, screenNum), xcol);
  }
  delete name;
  return xcol->pixel;
}

XFontStruct *ltkGetFontResouce(XrmDatabase db,
			       GString *appName, GString *appClass,
			       char *instName,
			       Display *display, int screenNum,
			       char *def) {
  GString *name;
  XFontStruct *fontStruct;

  name = ltkGetStringResource(db, appName, appClass, instName, def);
  if (!(fontStruct = XLoadQueryFont(display, name->getCString()))) {
    ltkError("Unknown font '%s'", name->getCString());
    if (!(fontStruct = XLoadQueryFont(display, LTK_DEF_FONT))) {
      ltkError("Can't find default font '%s' -- exiting", LTK_DEF_FONT);
      exit(1);
    }
  }
  delete name;
  return fontStruct;
}

void ltkGetGeometryResource(XrmDatabase db,
			    GString *appName, GString *appClass,
			    char *instName,
			    Display *display, int screenNum,
			    int *x, int *y,
			    Guint *width, Guint *height) {
  GString *geom;
  int flags;

  if ((geom = ltkGetStringResource(db, appName, appClass, instName, NULL))) {
    flags = XParseGeometry(geom->getCString(), x, y, width, height);
    delete geom;
    if ((flags & XValue) && (flags & XNegative))
      *x = DisplayWidth(display, screenNum) - *x;
    if ((flags & YValue) && (flags & YNegative))
      *y = DisplayHeight(display, screenNum) - *y;
  }
}
