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
#include "LTKWindow.h"
#include "LTKScrollingCanvas.h"

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
    w = width;
    if (left + width > realWidth) {
      if ((left = realWidth - width) < 0) {
	left = 0;
	w = realWidth;
      }
    }
    h = height;
    if (top + height > realHeight) {
      if ((top = realHeight - height) < 0) {
	top = 0;
	h = realHeight;
      }
    }
    XCopyArea(getDisplay(), pixmap, getXWindow(), getFgGC(),
	      left, top, w, h, 0, 0);
  }
}

void LTKScrollingCanvas::buttonPress(int mx, int my, int button,
				     GBool dblClick) {
  LTKWidget::buttonPress(left + mx, top + my, button, dblClick);
}

void LTKScrollingCanvas::buttonRelease(int mx, int my, int button,
				       GBool click) {
  LTKWidget::buttonRelease(left + mx, top + my, button, click);
}

void LTKScrollingCanvas::mouseMove(int mx, int my, int btn) {
  LTKWidget::mouseMove(left + mx, top + my, btn);
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
  int newLeft, newTop;
  int x1, y1, x2, y2, w, h;

  // compute new position
  newLeft = x;
  if (newLeft + width > realWidth) {
    newLeft = realWidth - width;
    if (newLeft < 0)
      newLeft = 0;
  }
  newTop = y;
  if (newTop + height > realHeight) {
    newTop = realHeight - height;
    if (newTop < 0)
      newTop = 0;
  }

  // check for no scrolling
  if (newLeft == left && newTop == top)
    return;

  // complete redraw
  if (left + width < newLeft || newLeft + width < left ||
      top + height < newTop || newTop + height < top) {
    w = (newLeft + width <= realWidth) ? width : realWidth;
    h = (newTop + height <= realHeight) ? height : realHeight;
    XCopyArea(getDisplay(), pixmap, getXWindow(), getFgGC(),
	      newLeft, newTop, w, h, 0, 0);

  // scroll window, then redraw edges
  } else {

    // copy as much as possible directly from window
    if (newLeft < left) {
      x1 = 0;
      x2 = left - newLeft;
      w = width - x2;
    } else {
      x1 = newLeft - left;
      x2 = 0;
      w = width - x1;
    }
    if (newTop < top) {
      y1 = 0;
      y2 = top - newTop;
      h = height - y2;
    } else {
      y1 = newTop - top;
      y2 = 0;
      h = height - y1;
    }
    XCopyArea(getDisplay(), getXWindow(), getXWindow(), getFgGC(),
	      x1, y1, w, h, x2, y2);

    // copy edges from pixmap
    h = (newTop + height <= realHeight) ? height : realHeight;
    if (newLeft < left) {
      w = left - newLeft;
      x1 = newLeft;
      x2 = 0;
    } else {
      w = newLeft - left;
      x1 = left + width;
      x2 = width - w;
    }
    if (w > 0) {
      XCopyArea(getDisplay(), pixmap, getXWindow(), getFgGC(),
		x1, newTop, w, h, x2, 0);
    }
    w = (newLeft + width <= realWidth) ? width : realWidth;
    if (newTop < top) {
      h = top - newTop;
      y1 = newTop;
      y2 = 0;
    } else {
      h = newTop - top;
      y1 = top + height;
      y2 = height - h;
    }
    if (h > 0) {
      XCopyArea(getDisplay(), pixmap, getXWindow(), getFgGC(),
		newLeft, y1, w, h, 0, y2);
    }
  }

  // update left and top
  left = newLeft;
  top = newTop;
}

void LTKScrollingCanvas::redrawRect(int x1, int y1, int x2, int y2) {
  int sx1, sy1, sx2, sy2;

  if (pixmap != None) {
    sx1 = x1 - left;
    sy1 = y1 - top;
    sx2 = x2 - left;
    sy2 = y2 - top;
    if (sx1 < width && sx2 >= 0 && sy1 < height && sy2 >= 0) {
      if (sx1 < 0)
	sx1 = 0;
      if (sy1 < 0)
	sy1 = 0;
      if (sx2 >= width)
	sx2 = width - 1;
      if (sy2 >= height)
	sy2 = height - 1;
      XCopyArea(getDisplay(), pixmap, getXWindow(), getFgGC(),
		left + sx1, top + sy1, sx2 - sx1 + 1, sy2 - sy1 + 1,
		sx1, sy1);
    }
  }
}
