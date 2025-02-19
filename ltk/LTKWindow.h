//========================================================================
//
// LTKWindow.h
//
// Copyright 1996-2002 Glyph & Cog, LLC
//
//========================================================================

#ifndef LTKWINDOW_H
#define LTKWINDOW_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include "gtypes.h"
#include "GString.h"

class LTKApp;
class LTKWidget;
class LTKBox;
class LTKWindow;
class LTKMenu;

//------------------------------------------------------------------------
// callback types
//------------------------------------------------------------------------

// Key press
typedef void (*LTKWindowKeyCbk)(LTKWindow *win, KeySym key, Guint modifiers,
				char *s, int n);

// Property change
typedef void (*LTKWindowPropCbk)(LTKWindow *win, Atom atom);

// Layout window
typedef void (*LTKWindowLayoutCbk)(LTKWindow *win);

//------------------------------------------------------------------------
// LTKWindow
//------------------------------------------------------------------------

class LTKWindow {
public:

  //---------- constructor and destructor ----------

  LTKWindow(LTKApp *appA, GBool dialogA, char *titleA,
	    char **iconDataA, char *defaultWidgetName,
	    LTKBox *boxA);

  ~LTKWindow();

  //---------- access ----------

  LTKApp *getApp() { return app; }
  int getWidth() { return width; }
  int getHeight() { return height; }
  GString *getSelection() { return selection; }
  Display *getDisplay() { return display; }
  int getScreenNum() { return screenNum; }
  Window getXWindow() { return xwin; }
  Colormap getColormap() { return colormap; }
  unsigned long getFgColor() { return fgColor; }
  unsigned long getBgColor() { return bgColor; }
  GC getFgGC() { return fgGC; }
  GC getBgGC() { return bgGC; }
  GC getBrightGC() { return brightGC; }
  GC getDarkGC() { return darkGC; }
  GC getXorGC() { return xorGC; }
  XFontStruct *getXFontStruct() { return fontStruct; }
  LTKWindow *getNext() { return next; }
  LTKWindow *setNext(LTKWindow *nextA) { return next = nextA; }
  LTKWidget *addWidget(LTKWidget *widget);
  LTKWidget *delWidget(LTKWidget *widget);
  LTKWidget *findWidget(char *name);

  //---------- special access ----------

  void setInstallCmap(GBool inst) { installCmap = inst; }
  void setMenu(LTKMenu *menuA) { menu = menuA; }
  void setKeyCbk(LTKWindowKeyCbk cbk) { keyCbk = cbk; }
  void setPropChangeCbk(LTKWindowPropCbk cbk);
  void setLayoutCbk(LTKWindowLayoutCbk cbk) { layoutCbk = cbk; }
  void setDecorated(GBool val) { decorated = val; }
  void setKeyWidget(LTKWidget *widget) { keyWidget = widget; }
  LTKWidget *getKeyWidget() { return keyWidget; }
  LTKWidget *getSelectionWidget() { return selectionWidget; }
  LTKWidget *getPasteWidget() { return pasteWidget; }
  LTKWindow *getOverWin() { return overWin; }

  //---------- layout ----------

  GBool checkFills(char **err);
  void layout(int x, int y, int widthA, int heightA);
  void layoutDialog(LTKWindow *overWinA, int widthA, int heightA);
  void map();

  //---------- drawing ----------

  void redraw();
  void redrawBackground();

  //---------- selection ----------

  void setSelection(LTKWidget *widget, GString *text);
  void requestPaste(LTKWidget *widget);

  //---------- callbacks and event handlers ----------

  void postMenu(int mx, int my);
  void keyPress(KeySym key, Guint modifiers, char *s, int n);
  void propChange(Atom atom);

  //---------- utility functions ----------

  // Create a new GC with specified parameters.
  GC makeGC(unsigned long color, int lineWidth, int lineStyle);

  // Set the window title.
  void setTitle(GString *title);

  // Set the cursor to one listed X11/cursorfont.h.
  void setCursor(Guint cursor);

  // Set the cursor to the default.
  void setDefaultCursor();

  // If busy is true, set the cursor to the "busy" cursor; otherwise
  // set it back to the previous cursor.  Calls to this function nest.
  void setBusyCursor(GBool busy);

  // Raise this window, deiconifying it if necessary.  Assumes map()
  // has already been called.
  void raise();

protected:

  LTKApp *app;			// application
  GBool dialog;			// dialog window?
  GString *title;		// window title
  char **iconData;		// data for icon pixmap
  int width, height;		// size of window
  LTKWidget *widgets;		// list of widgets (except boxes)
  LTKBox *box;			// contents of window

  GString *selection;		// selection text

  Guint savedCursor;		// cursor saved by setBusyCursor()
  int busyCursorNest;		// nesting level for setBusyCursor()

  LTKMenu *menu;		// the menu

  LTKWindowKeyCbk keyCbk;	// key press callback
  LTKWindowPropCbk propCbk;	// property change callback
  LTKWindowLayoutCbk layoutCbk;	// layout window callback

  GBool decorated;		// should this window be decorated?

  LTKWidget *defaultWidget;	// default widget (activated by return key)
  LTKWidget *keyWidget;		// current keyboard input focus
  LTKWidget *selectionWidget;	// widget which owns selection
  LTKWidget *pasteWidget;	// widget which gets next paste

  LTKWindow *overWin;		// if window is a dialog, this is the
				//   window it's placed over

  Display *display;		// X display
  int screenNum;		// X screen number
  Window xwin;			// X window ID
  long eventMask;		// current event mask
  GBool installCmap;		// install a private colormap
  Colormap colormap;		// the colormap
  unsigned long fgColor,	// foreground pixel number
                bgColor;	// background pixel number
  GC fgGC;			// X GC for foreground color
  GC bgGC;			// X GC for background color
  GC brightGC, darkGC;		// X GCs for shadow colors
  GC xorGC;			// X GC for cursors, highlighting, etc.
  XFontStruct *fontStruct;	// X font info

  LTKWindow *next;		// LTKApp keeps a list of windows
};

#endif
