//========================================================================
//
// LTKWindow.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKWINDOW_H
#define LTKWINDOW_H

#pragma interface

#include <X11/Xlib.h>
#include <stypes.h>
#include <String.h>

class LTKApp;
class LTKWidget;
class LTKBox;
class LTKWindow;

typedef void (*LTKWindowKeyCbk)(LTKWindow *win, KeySym key, char *s, int n);
typedef void (*LTKWindowPropCbk)(LTKWindow *win, Atom atom);

class LTKWindow {
public:

  LTKWindow(LTKApp *app1, char *title, LTKWindowKeyCbk keyCbk1,
	    LTKBox *box1);

  ~LTKWindow();

  LTKWindow *copy() { return new LTKWindow(this); }

  LTKApp *getApp() { return app; }
  int getWidth() { return width; }
  int getHeight() { return height; }

  Display *getDisplay() { return display; }
  int getScreenNum() { return screenNum; }
  Window getXWindow() { return xwin; }
  unsigned long getFgColor() { return fgColor; }
  unsigned long getBgColor() { return bgColor; }
  GC getFgGC() { return fgGC; }
  GC getBgGC() { return bgGC; }
  GC getBrightGC() { return brightGC; }
  GC getDarkGC() { return darkGC; }
  XFontStruct *getXFontStruct() { return fontStruct; }

  LTKWindow *getNext() { return next; }
  LTKWindow *setNext(LTKWindow *next1) { return next = next1; }

  LTKWidget *addWidget(LTKWidget *widget);
  LTKWidget *delWidget(LTKWidget *widget);
  LTKWidget *findWidget(Window xwin1);
  LTKWidget *findWidget(char *name);

  void setKeyWidget(LTKWidget *widget) { keyWidget = widget; }
  LTKWidget *getKeyWidget() { return keyWidget; }

  Boolean checkFills(char **err);

  void layout(int width1, int height1);

  void map();

  void redraw();

  // Set the property change callback.
  void setPropChangeCbk(LTKWindowPropCbk propCbk1);

  //---------- event handlers ----------

  void keyPress(KeySym key, char *s, int n);

  void doPropChange(Atom atom);

  //---------- utility functions ----------

  // Create a new GC with specified parameters.
  GC makeGC(unsigned long color, int lineWidth, int lineStyle);

  // Set the cursor to one listed X11/cursorfont.h.
  void setCursor(uint cursor);

protected:

  LTKWindow(LTKWindow *window);

  LTKApp *app;			// application
  String *title;		// window title
  int width, height;		// size of window
  LTKBox *box;			// contents of window
  LTKWidget *widgets;		// list of widgets (except boxes)
  LTKWindowKeyCbk keyCbk;	// key press callback
  LTKWindowPropCbk propCbk;	// property change callback

  LTKWidget *keyWidget;		// current keyboard input focus

  Display *display;		// X display
  int screenNum;		// X screen number
  Window xwin;			// X window ID
  long eventMask;		// requested input events
  unsigned long fgColor,	// foreground pixel number
                bgColor;	// background pixel number
  GC fgGC;			// X GC for foreground color
  GC bgGC;			// X GC for background color
  GC brightGC, darkGC;		// X GCs for shadow colors
  XFontStruct *fontStruct;	// X font info

  LTKWindow *next;		// LTKApp keeps a list of windows
};

#endif
