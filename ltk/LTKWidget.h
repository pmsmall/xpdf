//========================================================================
//
// LTKWidget.h
//
// Widget base class.
//
//========================================================================

#ifndef LTKWIDGET_H
#define LTKWIDGET_H

#pragma interface

#include <X11/Xlib.h>
#include <stypes.h>
#include <String.h>
#include <LTKWindow.h>

enum LTKWidgetKind {
  ltkBox,
  ltkButton,
  ltkCanvas,
  ltkDblBufCanvas,
  ltkEmpty,
  ltkLabel,
  ltkScrollbar,
  ltkScrollingCanvas,
  ltkTextIn
};

class LTKWidget {
public:

  //---------- constructor/destructor ----------

  // Constructor.
  LTKWidget(LTKWidgetKind kind1, char *name1);

  // Destructor.
  virtual ~LTKWidget();

  // Copy a widget.  Does not do any layout.
  virtual LTKWidget *copy() = 0;

  //---------- access ----------

  Boolean is(LTKWidgetKind kind1) { return kind == kind1; }
  String *getName() { return name; }
  virtual void setParent(LTKWindow *parent1);
  LTKWindow *getParent() { return parent; }
  int getWidth() { return width; }
  int getHeight() { return height; }
  Window getXWindow() { return xwin; }
  virtual long getEventMask() { return ExposureMask; }
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

  //---------- event handlers ----------

  virtual void buttonPress(int mx, int my, int button) {}
  virtual void buttonRelease(int mx, int my, int button) {}
  virtual void activate(Boolean on) {}
  virtual void mouseMove(int mx, int my) {}
  virtual void keyPress(KeySym key, char *s, int n) {}
  virtual void repeatEvent() {}

protected:

  // Make a copy of Widget fields only.
  LTKWidget(LTKWidget *widget):
    kind(widget->kind), name(widget->name ? widget->name->copy() : NULL),
    width(0), height(0) {}

  LTKWidgetKind kind;		// kind of widget
  String *name;			// name (for lookup within window)
  LTKWindow *parent;		// parent window
  int x, y;			// current position (in window)
  int width, height;		// current size

  Window xwin;			// X window (not used for Boxes)

  LTKWidget *next;		// LTKWindow keeps a list of widgets
};

#endif
