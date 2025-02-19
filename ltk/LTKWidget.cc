//========================================================================
//
// LTKWidget.cc
//
// Widget base class.
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <LTKWindow.h>
#include <LTKWidget.h>

LTKWidget::LTKWidget(char *name1, int widgetNum1) {
  name = name1;
  widgetNum = widgetNum1;
  x = 0;
  y = 0;
  width = 0;
  height = 0;
  btnPressCbk = NULL;
  btnReleaseCbk = NULL;
  xwin = None;
  next = NULL;
}

LTKWidget::LTKWidget(LTKWidget *widget) {
  name = widget->name;
  widgetNum = widget->widgetNum;
  x = 0;
  y = 0;
  width = 0;
  height = 0;
  btnPressCbk = NULL;
  btnReleaseCbk = NULL;
  xwin = None;
  next = NULL;
}

LTKWidget::~LTKWidget() {
}

void LTKWidget::setParent(LTKWindow *parent1) {
  parent = parent1;
  parent->addWidget(this);
}

long LTKWidget::getEventMask() {
  long mask;

  mask = ExposureMask;
  if (btnPressCbk)
    mask |= ButtonPressMask;
  if (btnReleaseCbk)
    mask |= ButtonReleaseMask;
  return mask;
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

void LTKWidget::buttonPress(int mx, int my, int button) {
  if (btnPressCbk)
    (*btnPressCbk)(this, widgetNum, mx, my, button);
}

void LTKWidget::buttonRelease(int mx, int my, int button) {
  if (btnReleaseCbk)
    (*btnPressCbk)(this, widgetNum, mx, my, button);
}
