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
#include <sys/types.h>
#include <sys/time.h>
#if defined(VMS) && defined(__DECCXX) && (__DECCXX_VER  < 50200000)
#include "vms_unix_time.h"
#endif
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include "gtypes.h"
#include "GString.h"

class LTKApp;
class LTKWindow;
class LTKWidget;
class LTKMenu;

//------------------------------------------------------------------------
// callback type
//------------------------------------------------------------------------

typedef void (*LTKAppKillCbk)(LTKWindow *win);

//------------------------------------------------------------------------
// LTKApp
//------------------------------------------------------------------------

#define ltkWinTabSize 127

struct LTKWinHash {
  Window xwin;
  LTKWindow *win;
  LTKWidget *widget;
  LTKWinHash *next;
};

class LTKApp {
public:

  //---------- constructor and destructor ----------

  LTKApp(char *appName1, XrmOptionDescRec *opts,
	 int *argc, char *argv[]);

  ~LTKApp();

  //---------- access ----------

  Display *getDisplay() { return display; }
  int getScreenNum() { return screenNum; }
  GString *getAppName() { return appName; }
  int getDisplayWidth() { return DisplayWidth(display, screenNum); }
  int getDisplayHeight() { return DisplayHeight(display, screenNum); }

  //---------- resources ----------

  GString *getStringResource(char *inst, char *def);
  int getIntResource(char *inst, int def);
  GBool getBoolResource(char *inst, GBool def);
  unsigned long getColorResource(char *inst,
				 char *def1, unsigned long def2,
				 XColor *xcol);
  XFontStruct *getFontResource(char *inst, char *def);
  void getGeometryResource(char *inst, int *x, int *y,
			   Guint *width, Guint *height);

  //---------- window list ----------

  LTKWindow *addWindow(LTKWindow *w);
  LTKWindow *delWindow(LTKWindow *w);
  void registerXWindow(Window xwin, LTKWindow *win, LTKWidget *widget);
  LTKWindow *findWindow(Window xwin, LTKWidget **widget);

  //---------- special access ----------

  void setGrabWin(LTKWindow *win) { grabWin = win; }
  void setMenu(LTKMenu *menu) { activeMenu = menu; }
  void setRepeatEvent(LTKWidget *repeatWidget1, int repeatDelay1,
		      int repeatPeriod1);
  void setKillCbk(LTKAppKillCbk cbk) { killCbk = cbk; }
  void clearButton() { pressedBtn = 0; }

  //---------- event handler ----------

  void doEvent(GBool wait);

private:

  GString *appName;		// application name (for X resources)
  LTKWindow *windows;		// list of windows
  LTKWinHash			// hash table of (X window) -> (LTK
    *winTab[ltkWinTabSize];	//   window/widget) mappings

  LTKWindow *grabWin;		// do events only for this window
  LTKMenu *activeMenu;		// currently posted menu

  LTKWidget *repeatWidget;	// do repeat events for this widget
  int repeatDelay;		// microseconds before first repeat event
  int repeatPeriod;		// microseconds between repeat events
  GBool firstRepeat;		// set before first repeat event
  struct timeval lastRepeat;	// time of last repeat

  Display *display;		// X display
  int screenNum;		// X screen number
  XrmDatabase resourceDB;	// X resource database
  Atom wmDeleteWinAtom;		// atom for WM_DELETE_WINDOW

  int pressedBtn;		// button currently pressed
  Time buttonPressTime;		// time of last button press

  LTKAppKillCbk killCbk;	// WM_DELETE_WINDOW callback
};

#endif
