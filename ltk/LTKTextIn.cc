//========================================================================
//
// LTKTextIn.cc
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
#include <X11/keysym.h>
#include <gtypes.h>
#include <GString.h>
#include <LTKWindow.h>
#include <LTKTextIn.h>
#include <LTKBorder.h>

#define horizBorder 2
#define vertBorder  2

LTKTextIn::LTKTextIn(char *name1, int widgetNum1, int minWidth1,
		     char *fontName1, LTKStringValCbk doneCbk1,
		     char *tabTarget1):
    LTKWidget(name1, widgetNum1) {
  minWidth = minWidth1;
  text = new GString();
  active = gFalse;
  firstChar = 0;
  cursor = 0;
  doneCbk = doneCbk1;
  tabTarget = tabTarget1;
  fontName = fontName1;
  fontStruct = NULL;
  textGC = None;
}

LTKTextIn::LTKTextIn(LTKTextIn *textIn):
    LTKWidget(textIn) {
  minWidth = textIn->minWidth;
  text = new GString();
  active = gFalse;
  cursor = 0;
  doneCbk = textIn->doneCbk;
  tabTarget = textIn->tabTarget;
  fontName = textIn->fontName;
  fontStruct = NULL;
  textGC = None;
}

LTKTextIn::~LTKTextIn() {
  delete text;
  if (fontName && fontStruct) {
    XFreeFont(getDisplay(), fontStruct);
    XFreeGC(getDisplay(), textGC);
  }
}

long LTKTextIn::getEventMask() {
  return LTKWidget::getEventMask() | ButtonPressMask | KeyPressMask;
}

void LTKTextIn::setText(char *s) {
  delete text;
  text = new GString(s);
  firstChar = 0;
  cursor = 0;
  if (getXWindow() != None) {
    redrawTail(0, gTrue);
    if (active)
      drawCursor(active);
  }
}

void LTKTextIn::layout1() {
  XGCValues gcValues;

  if (textGC == None) {
    if (fontName &&
	(fontStruct = XLoadQueryFont(getDisplay(), fontName))) {
      XGetGCValues(getDisplay(), getFgGC(),
		   GCForeground | GCBackground | GCGraphicsExposures,
		   &gcValues);
      gcValues.font = fontStruct->fid;
      textGC = XCreateGC(getDisplay(), parent->getXWindow(),
			 GCForeground | GCBackground | GCGraphicsExposures |
			 GCFont, &gcValues);
    } else {
      fontName = NULL;
      fontStruct = parent->getXFontStruct();
      textGC = getFgGC();
    }
  }
  textHeight = fontStruct->ascent + fontStruct->descent;
  textBase = fontStruct->ascent;
  width = minWidth * fontStruct->max_bounds.width + 2 * horizBorder + 1;
  height = textHeight + 2 * vertBorder;
}

void LTKTextIn::layout3() {
  XRectangle rect;

  LTKWidget::layout3();
  rect.x = horizBorder - 1;   // need one pixel in left border for cursor
  rect.y = 0;
  rect.width = width - 2 * horizBorder + 1;
  rect.height = height;
  XSetClipRectangles(getDisplay(), textGC, 0, 0, &rect, 1, Unsorted);
}

void LTKTextIn::redraw() {
  redrawTail(firstChar, gFalse);
  if (active)
    drawCursor(active);
}

void LTKTextIn::buttonPress(int mx, int my, int button) {
  XCharStruct extents;
  int direction, ascent, descent;
  int x1, x2;

  drawCursor(gFalse);
  x2 = horizBorder;
  for (cursor = firstChar; cursor < text->getLength(); ++cursor) {
    XTextExtents(fontStruct, text->getCString() + cursor, 1,
		 &direction, &ascent, &descent, &extents);
    x1 = x2;
    x2 += extents.width;
    if (mx < (x1 + x2) / 2)
      break;
  }
  drawCursor(gTrue);
}

void LTKTextIn::activate(GBool on) {
  if (active != on) {
    drawCursor(on);
    parent->setKeyWidget(on ? this : (LTKWidget *)NULL);
    active = on;
    if (!active && doneCbk)
      (*doneCbk)(this, widgetNum, text);
  }
}

void LTKTextIn::keyPress(KeySym key, char *s, int n) {
  int needRedraw;
  int oldCursor;
  LTKWidget *widget;

  drawCursor(gFalse);

  oldCursor = cursor;
  needRedraw = -1;
  if (key == XK_Left) {
    if (cursor > 0)
      --cursor;
  } else if (key == XK_Right) {
    if (cursor < text->getLength())
      ++cursor;
  } else if (n >= 1) {
    switch (s[0]) {
    case '\001':		// ^A
      cursor = 0;
      break;
    case '\002':		// ^B
      if (cursor > 0)
	--cursor;
      break;
    case '\005':		// ^E
      cursor = text->getLength();
      break;
    case '\006':		// ^F
      if (cursor < text->getLength())
	++cursor;
      break;
    case '\014':		// ^L
      needRedraw = firstChar;
      break;
    case '\b':			// bs
    case '\177':		// del
      if (cursor > 0) {
	text->del(cursor - 1);
	--cursor;
	needRedraw = cursor;
      }
      break;
    case '\004':		// ^D
      if (cursor < text->getLength()) {
	text->del(cursor);
	needRedraw = cursor;
      }
      break;
    case '\013':		// ^K
      text->del(cursor, text->getLength() - cursor);
      needRedraw = cursor;
      break;
    case '\025':		// ^U
      text->clear();
      cursor = 0;
      needRedraw = cursor;
      break;
    case '\n':
    case '\r':
    case '\t':
      activate(gFalse);
      if (tabTarget)
	if ((widget = parent->findWidget(tabTarget)))
	  widget->activate(gTrue);
      break;
    default:
      if (s[0] >= 0x20) {
	text->insert(cursor, s);
	needRedraw = cursor;
	++cursor;
      }
      break;
    }
  }

  if (needRedraw >= 0 || cursor != oldCursor)
    redrawTail(needRedraw, gTrue);
  if (active)
    drawCursor(gTrue);
}

void LTKTextIn::drawCursor(GBool on) {
  XCharStruct extents;
  int direction, ascent, descent;
  int tx, ty;

  tx = horizBorder;
  ty = (height - textHeight) / 2;
  if (cursor > firstChar) {
    XTextExtents(fontStruct, text->getCString() + firstChar,
		 cursor - firstChar,
		 &direction, &ascent, &descent, &extents);
    tx += extents.width;
  }
  if (on) {
    XDrawLine(getDisplay(), xwin, textGC, tx-1, ty, tx-1, ty + textHeight);
    XDrawLine(getDisplay(), xwin, textGC, tx, ty, tx, ty + textHeight);
  } else {
    XDrawLine(getDisplay(), xwin, getBgGC(), tx-1, ty, tx-1, ty + textHeight);
    XDrawLine(getDisplay(), xwin, getBgGC(), tx, ty, tx, ty + textHeight);
    ty += textBase;
    if (cursor <= firstChar) {
      XDrawString(getDisplay(), xwin, textGC, tx, ty,
		  text->getCString() + cursor, 1);
    } else {
      XTextExtents(fontStruct, text->getCString() + firstChar,
		   cursor - 1 - firstChar,
		   &direction, &ascent, &descent, &extents);
      tx = horizBorder + extents.width;
      XDrawString(getDisplay(), xwin, textGC, tx, ty,
		  text->getCString() + cursor - 1, 2);
    }
  }
}

void LTKTextIn::redrawTail(int i, GBool clear) {
  XCharStruct extents;
  int direction, ascent, descent;
  int tx, ty;

  // make sure cursor is visible
  if (cursor < firstChar) {
    firstChar = cursor;
    i = firstChar;
  } else if (cursor > firstChar) {
    XTextExtents(fontStruct, text->getCString() + firstChar,
		 cursor - firstChar,
		 &direction, &ascent, &descent, &extents);
    if (extents.width > width - 2 * horizBorder - 1) {
      do {
	++firstChar;
	XTextExtents(fontStruct, text->getCString() + firstChar,
		     cursor - firstChar,
		     &direction, &ascent, &descent, &extents);
      } while (extents.width > width - 2 * horizBorder - 1 &&
	       firstChar < cursor);
      i = firstChar;
    }
  }

  // redraw if necessary
  if (i >= 0) {
    tx = horizBorder;
    ty = (height - textHeight) / 2;
    if (i > firstChar) {
      XTextExtents(fontStruct, text->getCString() + firstChar, i - firstChar,
		   &direction, &ascent, &descent, &extents);
      tx += extents.width;
    }
    if (clear)
      XFillRectangle(getDisplay(), xwin, getBgGC(), tx, 0, width - tx, height);
    ty += textBase;
    XDrawString(getDisplay(), xwin, textGC, tx, ty,
		text->getCString() + i, text->getLength() - i);
  }
}
