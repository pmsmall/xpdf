//========================================================================
//
// LTKApp.h
//
//========================================================================

#ifndef LTKAPP_H
#define LTKAPP_H

#pragma interface

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <stypes.h>
#include <String.h>

class LTKWindow;
class LTKWidget;

class LTKApp {
public:

  LTKApp(char *appName1, XrmOptionDescRec *opts, int *argc, char *argv[]);

  ~LTKApp();

  Display *getDisplay() { return display; }
  int getScreenNum() { return screenNum; }

  String *getAppName() { return appName; }

  String *getStringResource(char *inst, char *def);
  unsigned long getColorResource(char *inst,
				 char *def1, unsigned long def2,
				 XColor *xcol);
  XFontStruct *LTKApp::getFontResource(char *inst,  char *def);

  LTKWindow *addWindow(LTKWindow *w);
  LTKWindow *delWindow(LTKWindow *w);
  LTKWindow *findWindow(Window xwin, LTKWidget **widget);

  void setRepeatEvent(LTKWidget *repeatWidget1)
    { repeatWidget = repeatWidget1; }

  void doEvent(Boolean wait);

private:

  String *appName;		// application name (for X resources)
  LTKWindow *windows;		// list of windows

  LTKWidget *repeatWidget;	// do repeat events for this widget

  Display *display;		// X display
  int screenNum;		// X screen number
  XrmDatabase resourceDB;	// X resource database;
};

#endif
