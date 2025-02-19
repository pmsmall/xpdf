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
#include <gtypes.h>
#include <GString.h>
#include <LTKWindow.h>

//------------------------------------------------------------------------
// callback types
//------------------------------------------------------------------------

// Mouse button press/release.
typedef void (*LTKMouseCbk)(LTKWidget *widget, int widgetNum,
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

  //---------- constructors and destructor ----------

  // Constructor.
  LTKWidget(char *name1, int widgetNum1);

  // Destructor.
  virtual ~LTKWidget();

  // Copy a widget.  Does not do any layout.
  virtual LTKWidget *copy() = 0;

  //---------- access ----------

  virtual GBool isBox() { return gFalse; }
  char *getName() { return name; }
  virtual void setParent(LTKWindow *parent1);
  LTKWindow *getParent() { return parent; }
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
  XFontStruct *getXFontStruct() { return parent->getXFontStruct(); }
  LTKWidget *getNext() { return next; }
  LTKWidget *setNext(LTKWidget *next1) { return next = next1; }

  //---------- special access ----------

  void setButtonPressCbk(LTKMouseCbk cbk) { btnPressCbk = cbk; }
  void setButtonReleaseCbk(LTKMouseCbk cbk) { btnReleaseCbk = cbk; }

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

  //---------- callbacks and event handlers ----------

  virtual void buttonPress(int mx, int my, int button);
  virtual void buttonRelease(int mx, int my, int button);
  virtual void activate(GBool on) {}
  virtual void mouseMove(int mx, int my) {}
  virtual void keyPress(KeySym key, char *s, int n) {}
  virtual void repeatEvent() {}

protected:

  // Make a copy of Widget fields only.
  LTKWidget(LTKWidget *widget);

  char *name;			// name (for lookup within window)
  int widgetNum;		// widget number (for callbacks)
  LTKWindow *parent;		// parent window
  int x, y;			// current position (in window)
  int width, height;		// current size

  LTKMouseCbk btnPressCbk;	// mouse button press callback
  LTKMouseCbk btnReleaseCbk;	// mouse button release callback

  Window xwin;			// X window (not used for Boxes)

  LTKWidget *next;		// LTKWindow keeps a list of widgets
};

#endif
