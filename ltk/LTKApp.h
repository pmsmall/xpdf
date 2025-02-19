//========================================================================
//
// LTKApp.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKAPP_H
#define LTKAPP_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <gtypes.h>
#include <GString.h>

class LTKWindow;
class LTKWidget;

//------------------------------------------------------------------------
// LTKApp
//------------------------------------------------------------------------

class LTKApp {
public:

  //---------- constructors and destructor ----------

  LTKApp(char *appName1, XrmOptionDescRec *opts, int *argc, char *argv[]);

  ~LTKApp();

  //---------- access ----------

  Display *getDisplay() { return display; }
  int getScreenNum() { return screenNum; }
  GString *getAppName() { return appName; }
  int getDisplayWidth() { return DisplayWidth(display, screenNum); }
  int getDisplayHeight() { return DisplayHeight(display, screenNum); }

  //---------- resources ----------

  GString *getStringResource(char *inst, char *def);
  unsigned long getColorResource(char *inst,
				 char *def1, unsigned long def2,
				 XColor *xcol);
  XFontStruct *LTKApp::getFontResource(char *inst,  char *def);
  void LTKApp::getGeometryResource(char *inst, int *x, int *y,
				   Guint *width, Guint *height);

  //---------- window list ----------

  LTKWindow *addWindow(LTKWindow *w);
  LTKWindow *delWindow(LTKWindow *w);
  LTKWindow *findWindow(Window xwin, LTKWidget **widget);

  //---------- special access ----------

  void setGrabWin(LTKWindow *win) { grabWin = win; }
  void setRepeatEvent(LTKWidget *repeatWidget1)
    { repeatWidget = repeatWidget1; }

  //---------- event handler ----------

  void doEvent(GBool wait);

private:

  GString *appName;		// application name (for X resources)
  LTKWindow *windows;		// list of windows

  LTKWindow *grabWin;		// do events only for this window
  LTKWidget *repeatWidget;	// do repeat events for this widget

  Display *display;		// X display
  int screenNum;		// X screen number
  XrmDatabase resourceDB;	// X resource database;
};

#endif
