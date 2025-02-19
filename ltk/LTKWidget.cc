//========================================================================
//
// LTKWidget.cc
//
// Widget base class.
//
//========================================================================

#pragma implementation

#include <stdlib.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <LTKWindow.h>
#include <LTKWidget.h>

LTKWidget::LTKWidget(LTKWidgetKind kind1, char *name1) {
  kind = kind1;
  name = name1 ? new String(name1) : NULL;
  x = 0;
  y = 0;
  width = 0;
  height = 0;
  xwin = None;
  next = NULL;
}

LTKWidget::~LTKWidget() {
  if (name)
    delete name;
}

void LTKWidget::setParent(LTKWindow *parent1) {
  parent = parent1;
  parent->addWidget(this);
}

void LTKWidget::layout2(int x1, int y1, int width1, int height1) {
  x = x1;
  y = y1;
  width = width1;
  height = height1;
}

void LTKWidget::layout3() {
  if (xwin == None) {
    xwin = XCreateSimpleWindow(getDisplay(), parent->getXWindow(),
			       x, y, width, height, 0,
			       getFgColor(), getBgColor());
    XSelectInput(getDisplay(), xwin, getEventMask());
  } else {
    XMoveResizeWindow(getDisplay(), xwin, x, y, width, height);
  }
}

void LTKWidget::map() {
  XMapWindow(getDisplay(), xwin);
}

void LTKWidget::clear() {
  XClearWindow(getDisplay(), xwin);
}
