//========================================================================
//
// LTKResources.cc
//
//========================================================================

#pragma implementation

#include <stdlib.h>
#include <stdio.h>
#include <stypes.h>
#include <String.h>
#include <LTKConfig.h>
#include <LTKMisc.h>
#include <LTKResources.h>

void ltkGetCmdLineResources(XrmDatabase *cmdLineDB, XrmOptionDescRec *opts,
			    int numOpts, String *appName,
			    int *argc, char *argv[]) {
  XrmParseCommand(cmdLineDB, opts, numOpts, appName->getCString(),
		  argc, argv);
}

void ltkGetOtherResources(Display *display,
			  XrmDatabase cmdLineDB, XrmDatabase *db) {
  XrmDatabase db1;
  String *s;
  FILE *f;

  if (XResourceManagerString(display)) {
    db1 = XrmGetStringDatabase(XResourceManagerString(display));
    XrmMergeDatabases(db1, db);
  } else {
    s = ltkGetHomeDir()->append("/.Xresources");
    if (f = fopen(s->getCString(), "r")) {
      fclose(f);
      XrmCombineFileDatabase(s->getCString(), db, True);
    }
    delete s;
    s = ltkGetHomeDir()->append("/.Xdefaults");
    if (f = fopen(s->getCString(), "r")) {
      fclose(f);
      XrmCombineFileDatabase(s->getCString(), db, True);
    }
    delete s;
  }
  XrmMergeDatabases(cmdLineDB, db);
}

String *ltkGetStringResource(XrmDatabase db, String *appName,
			     char *instName, char *def) {
  String *inst = appName->copy()->append(".")->append(instName);
  XrmValue val;
  char *resType[20];
  String *ret;

  if (XrmGetResource(db, inst->getCString(), inst->getCString(),
		     resType, &val))
    ret = new String(val.addr, val.size);
  else
    ret = def ? new String(def) : NULL;
  delete inst;
  return ret;
}

unsigned long ltkGetColorResource(XrmDatabase db, String *appName,
				  char *instName,
				  Display *display, int screenNum,
				  char *def1, unsigned long def2,
				  XColor *xcol) {
  String *name;
  XColor xcol1, xcol2;

  if (!xcol)
    xcol = &xcol1;
  name = ltkGetStringResource(db, appName, instName, def1);
  if (!XAllocNamedColor(display, DefaultColormap(display, screenNum),
			name->getCString(), xcol, &xcol2)) {
    ltkError("Unknown color '%s'", name->getCString());
    xcol->pixel = def2;
    XQueryColor(display, DefaultColormap(display, screenNum), xcol);
  }
  delete name;
  return xcol->pixel;
}

XFontStruct *ltkGetFontResouce(XrmDatabase db, String *appName,
			       char *instName,
			       Display *display, int screenNum,
			       char *def) {
  String *name;
  XFontStruct *fontStruct;

  name = ltkGetStringResource(db, appName, instName, def);
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
