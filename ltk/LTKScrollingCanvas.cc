//========================================================================
//
// LTKScrollingCanvas.cc
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
#include <LTKScrollingCanvas.h>

LTKScrollingCanvas::LTKScrollingCanvas(char *name1, int widgetNum1,
				       int realWidth1, int realHeight1,
				       int minWidth1, int minHeight1):
    LTKWidget(name1, widgetNum1) {
  realWidth = realWidth1;
  realHeight = realHeight1;
  minWidth = minWidth1;
  minHeight = minHeight1;
  left = top = 0;
  pixmap = None;
}

LTKScrollingCanvas::LTKScrollingCanvas(LTKScrollingCanvas *canvas):
    LTKWidget(canvas) {
  realWidth = canvas->realWidth;
  realHeight = canvas->realHeight;
  minWidth = canvas->minWidth;
  minHeight = canvas->minHeight;
  left = top = 0;
  pixmap = None;
}

LTKScrollingCanvas::~LTKScrollingCanvas() {
  if (pixmap != None)
    XFreePixmap(getDisplay(), pixmap);
}

void LTKScrollingCanvas::layout1() {
  width = minWidth;
  height = minHeight;
}

void LTKScrollingCanvas::layout3() {
  LTKWidget::layout3();
  if (pixmap == None) {
    pixmap = XCreatePixmap(getDisplay(), getXWindow(), realWidth, realHeight,
			   DefaultDepth(getDisplay(), getScreenNum()));
    XFillRectangle(getDisplay(), pixmap, getBgGC(),
		   0, 0, realWidth, realHeight);
  }
}

void LTKScrollingCanvas::redraw() {
  int w, h;

  if (pixmap != None) {
    w = (left + width) <= realWidth ? width : realWidth - left;
    h = (top + height) <= realHeight ? height : realHeight - top;
    XCopyArea(getDisplay(), pixmap, getXWindow(), getFgGC(),
	      left, top, w, h, 0, 0);
  }
}

void LTKScrollingCanvas::buttonPress(int mx, int my, int button) {
  if (btnPressCbk)
    (*btnPressCbk)(this, widgetNum, left + mx, top + my, button);
}

void LTKScrollingCanvas::resize(int realWidth1, int realHeight1) {
  if (realWidth1 != realWidth || realHeight1 != realHeight) {
    if (pixmap != None)
      XFreePixmap(getDisplay(), pixmap);
    realWidth = realWidth1;
    realHeight = realHeight1;
    pixmap = XCreatePixmap(getDisplay(), getXWindow(), realWidth, realHeight,
			   DefaultDepth(getDisplay(), getScreenNum()));
    XFillRectangle(getDisplay(), pixmap, getBgGC(),
		   0, 0, realWidth, realHeight);
    XClearWindow(getDisplay(), getXWindow());
  }
}

void LTKScrollingCanvas::scroll(int x, int y) {
  left = x;
  top = y;
  if (left + width > realWidth && left > 0)
    left = 0;
  if (top + height > realHeight && top > 0)
    top = 0;
  redraw();
}
