//========================================================================
//
// LTKScrollbar.cc
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
#include "LTKApp.h"
#include "LTKWindow.h"
#include "LTKScrollbar.h"
#include "LTKBorder.h"

//------------------------------------------------------------------------

#define ltkScrollbarRepeatDelay  400000  // delay (us) before first repeat

#define ltkScrollbarRepeatPeriod  30000  // default time (us) between
				         //   repeats

//------------------------------------------------------------------------
// LTKScrollbar
//------------------------------------------------------------------------

LTKScrollbar::LTKScrollbar(char *name1, int widgetNum1,
			   GBool vertical1, int minPos1, int maxPos1,
			   LTKIntValCbk moveCbk1):
    LTKWidget(name1, widgetNum1) {
  vertical = vertical1;
  minPos = minPos1;
  maxPos = maxPos1;
  moveCbk = moveCbk1;

  pos = minPos;
  if ((size = (maxPos - minPos) / 8) == 0)
    size = 1;
  scrollDelta = -1;
  repeatPeriod = ltkScrollbarRepeatPeriod;
  sliderPressed = gFalse;
  upPressed = downPressed = gFalse;
}

long LTKScrollbar::getEventMask() {
  return LTKWidget::getEventMask() |
         ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
         PointerMotionHintMask;
}

void LTKScrollbar::layout1() {
  if (vertical) {
    width = 12 + 2 * ltkBorderWidth;
    height = 40 + 2 * ltkBorderWidth;
  } else {
    width = 40 + 2 * ltkBorderWidth;
    height = 12 + 2 * ltkBorderWidth;
  }
}

void LTKScrollbar::redraw() {
  ltkDrawBorder(getDisplay(), xwin, getBrightGC(), getDarkGC(), getBgGC(),
		0, 0, width, height, ltkBorderSunken);
  drawUpButton();
  drawDownButton();
  drawSlider(pos, gTrue);
}

void LTKScrollbar::setLimits(int minPos1, int maxPos1) {
  if (getXWindow() != None)
    drawSlider(pos, gFalse);
  minPos = minPos1;
  maxPos = maxPos1;
  if (maxPos < minPos)
    maxPos = minPos;
  if (size > maxPos - minPos + 1)
    size = maxPos - minPos + 1;
  if (pos < minPos)
    pos = minPos;
  else if (pos > maxPos - size + 1)
    pos = maxPos - size + 1;
  if (getXWindow() != None)
    drawSlider(pos, gTrue);
}

void LTKScrollbar::setPos(int pos1, int size1) {
  if (getXWindow() != None)
    drawSlider(pos, gFalse);
  pos = pos1;
  size = size1;
  if (size > maxPos - minPos + 1)
    size = maxPos - minPos + 1;
  if (pos < minPos)
    pos = minPos;
  else if (pos > maxPos - size + 1)
    pos = maxPos - size + 1;
  if (getXWindow() != None)
    drawSlider(pos, gTrue);
}

void LTKScrollbar::drawUpButton() {
  if (vertical)
    ltkDrawTriBorder(getDisplay(), xwin, getBrightGC(), getDarkGC(), getBgGC(),
		     ltkBorderWidth+1, ltkBorderWidth+1, 10, 10,
		     ltkTriUp,
		     upPressed ? ltkBorderSunken : ltkBorderRaised);
  else
    ltkDrawTriBorder(getDisplay(), xwin, getBrightGC(), getDarkGC(), getBgGC(),
		     ltkBorderWidth+1, ltkBorderWidth+1, 10, 10,
		     ltkTriLeft,
		     upPressed ? ltkBorderSunken : ltkBorderRaised);
}

void LTKScrollbar::drawDownButton() {
  if (vertical)
    ltkDrawTriBorder(getDisplay(), xwin, getBrightGC(), getDarkGC(), getBgGC(),
		     ltkBorderWidth+1, height-ltkBorderWidth-11, 10, 10,
		     ltkTriDown,
		     downPressed ? ltkBorderSunken : ltkBorderRaised);
  else
    ltkDrawTriBorder(getDisplay(), xwin, getBrightGC(), getDarkGC(), getBgGC(),
		     width-ltkBorderWidth-11, ltkBorderWidth+1, 10, 10,
		     ltkTriRight,
		     downPressed ? ltkBorderSunken : ltkBorderRaised);
}

void LTKScrollbar::drawSlider(int pos1, Bool on) {
  int w1, h1;
  int tx, ty, tw, th;
  LTKBorder border;

  if (vertical) {
    h1 = height - 2 * ltkBorderWidth - 24;
    th = (size * h1) / (maxPos - minPos + 1);
    ty = ltkBorderWidth + 12;
    if (minPos < maxPos)
      ty += ((pos1 - minPos) * h1) / (maxPos - minPos + 1);
    if (th < 16)
      th = 16;
    if (ty + th > height - ltkBorderWidth - 12)
      ty = height - ltkBorderWidth - 12 - th;
    tw = width - 2 * ltkBorderWidth;
    tx = ltkBorderWidth;
    pixelPos = ty;
    pixelSize = th;
  } else {
    w1 = width - 2 * ltkBorderWidth - 24;
    tw = (size * w1) / (maxPos - minPos + 1);
    tx = ltkBorderWidth + 12;
    if (minPos < maxPos)
      tx += ((pos1 - minPos) * w1) / (maxPos - minPos + 1);
    else
      tx = ltkBorderWidth;
    if (tw < 16)
      tw = 16;
    if (tx + tw > width - ltkBorderWidth - 12)
      tx = width - ltkBorderWidth - 12 - tw;
    th = height - 2 * ltkBorderWidth;
    ty = ltkBorderWidth;
    pixelPos = tx;
    pixelSize = tw;
  }
  if (on)
    border = sliderPressed ? ltkBorderSunken : ltkBorderRaised;
  else
    border = ltkBorderNone;
  ltkDrawBorder(getDisplay(), xwin, getBrightGC(), getDarkGC(), getBgGC(),
		tx, ty, tw, th, border);
}

void LTKScrollbar::buttonPress(int mx, int my, int button, GBool dblClick) {
  int oldPos;

  // scroll up/left
  if ((vertical && my < ltkBorderWidth + 11) ||
      (!vertical && mx < ltkBorderWidth + 11)) {
    upPressed = gTrue;
    drawUpButton();
    doScroll();
    //~ user-settable period
    parent->getApp()->setRepeatEvent(this, ltkScrollbarRepeatDelay,
				     repeatPeriod);

  // page up/left
  } else if ((vertical && my < pixelPos) ||
	     (!vertical && mx < pixelPos)) {
    oldPos = pos;
    if ((pos -= size) < minPos)
      pos = minPos;
    if (pos != oldPos) {
      drawSlider(oldPos, gFalse);
      drawSlider(pos, gTrue);
      (*moveCbk)(this, widgetNum, pos);
    }

  // drag
  } else if ((vertical && my < pixelPos + pixelSize) ||
	     (!vertical && mx < pixelPos + pixelSize)) {
    pushPixel = vertical ? my : mx;
    pushPos = pos;
    sliderPressed = gTrue;
    drawSlider(pos, gTrue);

  // page down/right
  } else if ((vertical && my < height - ltkBorderWidth - 11) ||
	     (!vertical && mx < width - ltkBorderWidth - 11)) {
    oldPos = pos;
    if ((pos += size) > maxPos - size + 1)
      pos = maxPos - size + 1;
    if (pos != oldPos) {
      drawSlider(oldPos, gFalse);
      drawSlider(pos, gTrue);
      (*moveCbk)(this, widgetNum, pos);
    }

  // scroll down/right
  } else {
    downPressed = gTrue;
    drawDownButton();
    doScroll();
    //~ user-settable period
    parent->getApp()->setRepeatEvent(this, ltkScrollbarRepeatDelay,
				     repeatPeriod);
  }
}

void LTKScrollbar::buttonRelease(int mx, int my, int button, GBool click) {
  if (sliderPressed) {
    sliderPressed = gFalse;
    drawSlider(pos, gTrue);
  } else if (upPressed) {
    upPressed = gFalse;
    drawUpButton();
    parent->getApp()->setRepeatEvent(NULL, 0, 0);
  } else if (downPressed) {
    downPressed = gFalse;
    drawDownButton();
    parent->getApp()->setRepeatEvent(NULL, 0, 0);
  }
}

void LTKScrollbar::mouseMove(int mx, int my, int btn) {
  int oldPos = pos;
  int w1, h1;

  if (sliderPressed) {
    if (vertical) {
      h1 = height - 2 * ltkBorderWidth - 24;
      pos = pushPos + ((my - pushPixel) * (maxPos - minPos + 1)) / h1;
    } else {
      w1 = width - 2 * ltkBorderWidth - 24;
      pos = pushPos + ((mx - pushPixel) * (maxPos - minPos + 1)) / w1;
    }
    if (pos < minPos)
      pos = minPos;
    else if (pos > maxPos - size + 1)
      pos = maxPos - size + 1;
    if (pos != oldPos) {
      drawSlider(oldPos, gFalse);
      drawSlider(pos, gTrue);
      (*moveCbk)(this, widgetNum, pos);
    }
  }
}

void LTKScrollbar::repeatEvent() {
  doScroll();
}

void LTKScrollbar::doScroll() {
  int oldPos;
  int size1;

  oldPos = pos;
  if (upPressed) {
    if (scrollDelta > 0) {
      pos -= scrollDelta;
    } else {
      size1 = (vertical ? height : width) - 2 * ltkBorderWidth - 24;
      if (maxPos - minPos + 1 > size1)
	pos -= (maxPos - minPos + 1) / size1;
      else
	--pos;
    }
    if (pos < minPos)
      pos = minPos;
    if (pos != oldPos) {
      drawSlider(oldPos, gFalse);
      drawSlider(pos, gTrue);
      (*moveCbk)(this, widgetNum, pos);
    }
  } else if (downPressed) {
    if (scrollDelta > 0) {
      pos += scrollDelta;
    } else {
      size1 = (vertical ? height : width) - 2 * ltkBorderWidth - 24;
      if (maxPos - minPos + 1 > size1)
	pos += (maxPos - minPos + 1) / size1;
      else
	++pos;
    }
    if (pos > maxPos - size + 1)
      pos = maxPos - size + 1;
    if (pos != oldPos) {
      drawSlider(oldPos, gFalse);
      drawSlider(pos, gTrue);
      (*moveCbk)(this, widgetNum, pos);
    }
  }
}
