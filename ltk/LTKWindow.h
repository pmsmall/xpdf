//========================================================================
//
// LTKWindow.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKWINDOW_H
#define LTKWINDOW_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include <gtypes.h>
#include <GString.h>

class LTKApp;
class LTKWidget;
class LTKBox;
class LTKWindow;

//------------------------------------------------------------------------
// callback types
//------------------------------------------------------------------------

// Key press
typedef void (*LTKWindowKeyCbk)(LTKWindow *win, KeySym key, char *s, int n);

// Property change
typedef void (*LTKWindowPropCbk)(LTKWindow *win, Atom atom);

// Layout window
typedef void (*LTKWindowLayoutCbk)(LTKWindow *win);

//------------------------------------------------------------------------
// LTKWindow
//------------------------------------------------------------------------

class LTKWindow {
public:

  //---------- constructors and destructor ----------

  LTKWindow(LTKApp *app1, GBool dialog1, char *title,
	    LTKBox *box1);

  ~LTKWindow();

  LTKWindow *copy() { return new LTKWindow(this); }

  //---------- access ----------

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

  //---------- special access ----------

  void setKeyCbk(LTKWindowKeyCbk cbk);
  void setPropChangeCbk(LTKWindowPropCbk cbk);
  void setLayoutCbk(LTKWindowLayoutCbk cbk) { layoutCbk = cbk; }
  void setKeyWidget(LTKWidget *widget) { keyWidget = widget; }
  LTKWidget *getKeyWidget() { return keyWidget; }
  LTKWindow *getOverWin() { return overWin; }

  //---------- layout ----------

  GBool checkFills(char **err);
  void layout(int x, int y, int width1, int height1);
  void layoutDialog(LTKWindow *overWin1, int width1, int height1);
  void map();

  //---------- drawing ----------

  void redraw();

  //---------- callbacks and event handlers ----------

  void keyPress(KeySym key, char *s, int n);
  void propChange(Atom atom);

  //---------- utility functions ----------

  // Create a new GC with specified parameters.
  GC makeGC(unsigned long color, int lineWidth, int lineStyle);

  // Set the window title.
  void setTitle(GString *title1);

  // Set the cursor to one listed X11/cursorfont.h.
  void setCursor(Guint cursor);

protected:

  LTKWindow(LTKWindow *window);

  LTKApp *app;			// application
  GBool dialog;			// dialog window?
  GString *title;		// window title
  int width, height;		// size of window
  LTKWidget *widgets;		// list of widgets (except boxes)
  LTKBox *box;			// contents of window

  LTKWindowKeyCbk keyCbk;	// key press callback
  LTKWindowPropCbk propCbk;	// property change callback
  LTKWindowLayoutCbk layoutCbk;	// layout window callback

  LTKWidget *keyWidget;		// current keyboard input focus

  LTKWindow *overWin;		// if window is a dialog, this is the
				//   window it's placed over

  Display *display;		// X display
  int screenNum;		// X screen number
  Window xwin;			// X window ID
  long eventMask;		// current event mask
  unsigned long fgColor,	// foreground pixel number
                bgColor;	// background pixel number
  GC fgGC;			// X GC for foreground color
  GC bgGC;			// X GC for background color
  GC brightGC, darkGC;		// X GCs for shadow colors
  XFontStruct *fontStruct;	// X font info

  LTKWindow *next;		// LTKApp keeps a list of windows
};

#endif
