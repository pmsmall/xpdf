//========================================================================
//
// LTKWidget.h
//
// Widget base class.
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKWIDGET_H
#define LTKWIDGET_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include "gtypes.h"
#include "GString.h"
#include "LTKWindow.h"

//------------------------------------------------------------------------
// callback types
//------------------------------------------------------------------------

// Mouse button press/release.
typedef void (*LTKButtonPressCbk)(LTKWidget *widget, int widgetNum,
				  int mx, int my, int button, GBool dblClick);
typedef void (*LTKButtonReleaseCbk)(LTKWidget *widget, int widgetNum,
				    int mx, int my, int button, GBool click);

// Mouse move.
typedef void (*LTKMouseMoveCbk)(LTKWidget *widget, int widgetNum,
				int mx, int my);

// Mouse drag.
typedef void (*LTKMouseDragCbk)(LTKWidget *widget, int widgetNum,
				int mx, int my, int button);

// Boolean value change.
typedef void (*LTKBoolValCbk)(LTKWidget *widget, int widgetNum, GBool on);

// Integer value change.
typedef void (*LTKIntValCbk)(LTKWidget *widget, int widgetNum, int val);

// String value change.
typedef void (*LTKStringValCbk)(LTKWidget *widget, int widgetNum,
				GString *val);

// Redraw widget.
typedef void (*LTKRedrawCbk)(LTKWidget *widget, int widgetNum);

//------------------------------------------------------------------------
// LTKWidget
//------------------------------------------------------------------------

class LTKWidget {
public:

  //---------- constructor and destructor ----------

  // Constructor.
  LTKWidget(char *name1, int widgetNum1);

  // Destructor.
  virtual ~LTKWidget();

  //---------- access ----------

  virtual GBool isBox() { return gFalse; }
  char *getName() { return name; }
  virtual void setParent(LTKWindow *parent1);
  LTKWindow *getParent() { return parent; }
  virtual void setCompoundParent(LTKWidget *compParent1);
  LTKWidget *getCompoundParent() { return compParent; }
  int getWidth() { return width; }
  int getHeight() { return height; }
  Window getXWindow() { return xwin; }
  virtual long getEventMask();
  Display *getDisplay() { return parent->getDisplay(); }
  int getScreenNum() { return parent->getScreenNum(); }
  unsigned long getFgColor() { return parent->getFgColor(); }
  unsigned long getBgColor() { return parent->getBgColor(); }
  GC getFgGC() { return parent->getFgGC(); }
  GC getBgGC() { return parent->getBgGC(); }
  GC getBrightGC() { return parent->getBrightGC(); }
  GC getDarkGC() { return parent->getDarkGC(); }
  GC getXorGC() { return parent->getXorGC(); }
  XFontStruct *getXFontStruct() { return parent->getXFontStruct(); }
  LTKWidget *getNext() { return next; }
  LTKWidget *setNext(LTKWidget *next1) { return next = next1; }

  //---------- special access ----------

  void setButtonPressCbk(LTKButtonPressCbk cbk) { btnPressCbk = cbk; }
  void setButtonReleaseCbk(LTKButtonReleaseCbk cbk) { btnReleaseCbk = cbk; }
  void setMouseMoveCbk(LTKMouseMoveCbk cbk) { mouseMoveCbk = cbk; }
  void setMouseDragCbk(LTKMouseDragCbk cbk) { mouseDragCbk = cbk; }

  //---------- layout ----------

  // Compute minimum width and height of widget.
  virtual void layout1() = 0;

  // Layout widget internals at specified position, with specified size.
  virtual void layout2(int x1, int y1, int width1, int height1);

  // Construct the X window(s) for widget and children.  If windows
  // are already constructed, move/resize them as necessary.
  virtual void layout3();

  // Map the X window(s) for widget and children.
  virtual void map();

  //---------- drawing ----------

  // Clear the widget's window to the background color.
  virtual void clear();

  // Draw the widget and its children.
  virtual void redraw() = 0;

  // Draw the window background (only used by boxes, compound widgets,
  // etc., which draw directly in the window).
  virtual void redrawBackground() {}

  //---------- callbacks and event handlers ----------

  virtual void buttonPress(int mx, int my, int button, GBool dblClick);
  virtual void buttonRelease(int mx, int my, int button, GBool click);
  virtual void activate(GBool on) {}
  virtual void activateDefault() {}
  virtual void mouseMove(int mx, int my, int btn);
  virtual void keyPress(KeySym key, Guint modifiers, char *s, int n) {}
  virtual void repeatEvent() {}
  virtual void clearSelection() {}
  virtual void paste(GString *str) {}

protected:

  char *name;			// name (for lookup within window)
  int widgetNum;		// widget number (for callbacks)
  LTKWindow *parent;		// parent window
  LTKWidget *compParent;	// parent compound widget
  int x, y;			// current position (in window)
  int width, height;		// current size

  LTKButtonPressCbk btnPressCbk;     // mouse button press callback
  LTKButtonReleaseCbk btnReleaseCbk; // mouse button release callback
  LTKMouseMoveCbk mouseMoveCbk;	     // mouse move callback
  LTKMouseDragCbk mouseDragCbk;	     // mouse drag callback

  Window xwin;			// X window (not used for Boxes)
  GBool mapped;			// set when mapped

  LTKWidget *next;		// LTKWindow keeps a list of widgets
};

#endif
