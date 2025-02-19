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
#include "gtypes.h"
#include "GString.h"
#include "LTKWindow.h"
#include "LTKTextIn.h"
#include "LTKBorder.h"

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
  selectionEnd = 0;
  dragging = gFalse;
  doneCbk = doneCbk1;
  tabTarget = tabTarget1;
  fontName = fontName1;
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
  return LTKWidget::getEventMask() |
         ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
         PointerMotionHintMask | KeyPressMask;
}

void LTKTextIn::setText(char *s) {
  delete text;
  text = new GString(s);
  firstChar = 0;
  if (getXWindow() != None && active)
    xorCursor();
  cursor = selectionEnd = 0;
  if (getXWindow() != None) {
    redrawTail(0);
    if (active)
      xorCursor();
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
  XFillRectangle(getDisplay(), xwin, getBgGC(), 0, 0, width, height);
  redrawTail(firstChar);
  if (active)
    xorCursor();
}

void LTKTextIn::buttonPress(int mx, int my, int button, GBool dblClick) {
  // move cursor
  if (button == 1) {
    xorCursor();
    cursor = xToCursor(mx);
    if (cursor < firstChar)
      cursor = firstChar;
    selectionEnd = cursor;
    xorCursor();
    dragging = gTrue;
    dragAnchor = cursor;

  // paste
  } else if (button == 2) {
    parent->requestPaste(this);
  }
}

void LTKTextIn::buttonRelease(int mx, int my, int button, GBool click) {
  dragging = gFalse;
  if (cursor != selectionEnd)
    parent->setSelection(this, new GString(text->getCString() + cursor,
					   selectionEnd - cursor));
}

void LTKTextIn::mouseMove(int mx, int my, int btn) {
  int newCursor, newSelectionEnd;
  int i;

  if (dragging) {
    i = xToCursor(mx);
    if (i >= dragAnchor) {
      newCursor = dragAnchor;
      newSelectionEnd = i;
    } else {
      newCursor = i;
      newSelectionEnd = dragAnchor;
    }
    if (newCursor != cursor || newSelectionEnd != selectionEnd)
      moveCursor(newCursor, newSelectionEnd, i);
  }
}

void LTKTextIn::activate(GBool on) {
  if (active != on) {
    if (on) {
      xorCursor();
      parent->setKeyWidget(this);
      active = gTrue;
    } else {
      xorCursor();
      selectionEnd = cursor;
      parent->setKeyWidget(NULL);
      active = gFalse;
      if (doneCbk)
	(*doneCbk)(this, widgetNum, text);
    }
  }
}

void LTKTextIn::keyPress(KeySym key, Guint modifiers, char *s, int n) {
  int newCursor, newSelectionEnd;
  int redrawPos;
  GBool killSelection;
  LTKWidget *widget;

  newCursor = cursor;
  killSelection = gTrue;
  redrawPos = -1;
  if (key == XK_Left) {		// left arrow
    if (cursor > 0)
      --newCursor;
  } else if (key == XK_Right) {	// right arrow
    if (cursor < text->getLength())
      ++newCursor;
  } else if (n >= 1) {
    switch (s[0]) {
    case '\001':		// ^A
      newCursor = 0;
      break;
    case '\002':		// ^B
      if (cursor > 0)
	--newCursor;
      break;
    case '\005':		// ^E
      newCursor = text->getLength();
      break;
    case '\006':		// ^F
      if (cursor < text->getLength())
	++newCursor;
      break;
    case '\014':		// ^L
      redrawPos = firstChar;
      killSelection = gFalse;
      break;
    case '\b':			// bs
    case '\177':		// del
      if (selectionEnd > cursor) {
	text->del(cursor, selectionEnd - cursor);
	redrawPos = cursor;
      } else if (cursor > 0) {
	text->del(cursor - 1);
	--newCursor;
	redrawPos = newCursor;
      }
      break;
    case '\004':		// ^D
      if (selectionEnd > cursor) {
	text->del(cursor, selectionEnd - cursor);
	redrawPos = cursor;
      } else if (cursor < text->getLength()) {
	text->del(cursor);
	redrawPos = cursor;
      }
      break;
    case '\013':		// ^K
      text->del(cursor, text->getLength() - cursor);
      redrawPos = cursor;
      break;
    case '\025':		// ^U
      text->clear();
      newCursor = 0;
      redrawPos = newCursor;
      break;
    case '\n':			// return, tab
    case '\r':
    case '\t':
      activate(gFalse);
      if (tabTarget)
	if ((widget = parent->findWidget(tabTarget)))
	  widget->activate(gTrue);
      newCursor = newSelectionEnd = cursor;
      break;
    default:			// insert char
      if (s[0] >= 0x20) {
	if (selectionEnd > cursor)
	  text->del(cursor, selectionEnd - cursor);
	text->insert(cursor, s);
	redrawPos = cursor;
	++newCursor;
      }
      break;
    }
  } else {			// ignore weird X keysyms
    killSelection = gFalse;
  }

  newSelectionEnd = killSelection ? newCursor : selectionEnd;
  if (newCursor != cursor || newSelectionEnd != selectionEnd ||
      redrawPos >= 0) {
    xorCursor();
    cursor = newCursor;
    selectionEnd = newSelectionEnd;
    redrawTail(redrawPos);
    if (active)
      xorCursor();
  }
}

void LTKTextIn::clearSelection() {
  if (active)
    xorCursor();
  selectionEnd = cursor;
  if (active)
    xorCursor();
}

void LTKTextIn::paste(GString *str) {
  int redrawPos;

  if (active)
    xorCursor();
  text->insert(cursor, str);
  redrawPos = cursor;
  cursor += str->getLength();
  selectionEnd = cursor;
  redrawTail(redrawPos);
  if (active)
    xorCursor();
}

int LTKTextIn::xToCursor(int mx) {
  XCharStruct extents;
  int direction, ascent, descent;
  int x1, x2;
  int pos;

  if (mx < horizBorder)
    return firstChar > 0 ? firstChar - 1 : 0;

  x2 = horizBorder;
  for (pos = firstChar; pos < text->getLength(); ++pos) {
    XTextExtents(fontStruct, text->getCString() + pos, 1,
		 &direction, &ascent, &descent, &extents);
    x1 = x2;
    x2 += extents.width;
    if (mx < (x1 + x2) / 2)
      break;
  }
  return pos;
}

int LTKTextIn::cursorToX(int cur) {
  XCharStruct extents;
  int direction, ascent, descent;
  int x;

  x = horizBorder;
  if (cur > firstChar) {
    XTextExtents(fontStruct, text->getCString() + firstChar,
		 cur - firstChar,
		 &direction, &ascent, &descent, &extents);
    x += extents.width;
  }
  return x;
}

void LTKTextIn::xorCursor() {
  int tx1, tx2, ty;
  int i, j;

  ty = (height - textHeight) / 2;

  // draw cursor
  if (cursor == selectionEnd) {
    tx1 = cursorToX(cursor);
    XDrawLine(getDisplay(), xwin, getXorGC(),
	      tx1 - 1, ty, tx1 - 1, ty + textHeight - 1);
    XDrawLine(getDisplay(), xwin, getXorGC(),
	      tx1, ty, tx1, ty + textHeight - 1);

  // draw selection
  } else {
    i = (cursor >= firstChar) ? cursor : firstChar;
    tx1 = cursorToX(i);
    j = selectionEnd;
    tx2 = cursorToX(j);
    if (tx2 > width) {
      tx2 = width;
      j = xToCursor(tx2);
      if (j < text->getLength())
	++j;
    }
    XFillRectangle(getDisplay(), xwin, getXorGC(),
		   tx1, ty, tx2 - tx1, textHeight);
  }
}

void LTKTextIn::moveCursor(int newCursor, int newSelectionEnd,
			   int visiblePos) {
  GBool needRedraw;
  int tx1, tx2, ty;
  int i;

  // y coordinate
  ty = (height - textHeight) / 2;

  // make sure moving end of selection is visible
  needRedraw = gFalse;
  if (visiblePos < firstChar) {
    firstChar = visiblePos;
    needRedraw = gTrue;
  } else if (visiblePos > firstChar) {
    tx1 = cursorToX(visiblePos);
    if (tx1 > width - horizBorder - 1) {
      do {
	++firstChar;
	tx1 = cursorToX(visiblePos);
      } while (tx1 > width - horizBorder - 1 && firstChar < visiblePos);
      needRedraw = gTrue;
    }
  }
  if (needRedraw) {
    tx1 = horizBorder;
    XFillRectangle(getDisplay(), xwin, getBgGC(),
		   tx1, ty, width - tx1, textHeight);
    i = xToCursor(width);
    if (i < text->getLength())
      ++i;
    XDrawString(getDisplay(), xwin, textGC, tx1, ty + textBase,
		text->getCString() + firstChar, i - firstChar);
    cursor = newCursor;
    selectionEnd = newSelectionEnd;
    xorCursor();
  } else {

    // no selection previously
    if (cursor == selectionEnd) {
      xorCursor();

    } else {

      // shrank on the left
      if (newCursor > cursor) {
	tx1 = cursorToX(cursor);
	tx2 = cursorToX(newCursor);
	XFillRectangle(getDisplay(), xwin, getXorGC(),
		       tx1, ty, tx2 - tx1, textHeight);
      }

      // shrank on the right
      if (newSelectionEnd  < selectionEnd) {
	tx1 = cursorToX(newSelectionEnd);
	tx2 = cursorToX(selectionEnd);
	XFillRectangle(getDisplay(), xwin, getXorGC(),
		       tx1, ty, tx2 - tx1, textHeight);
      }
    }

    // no selection left
    if (newCursor == newSelectionEnd) {
      cursor = newCursor;
      selectionEnd = newSelectionEnd;
      xorCursor();

    } else {

      // grew on the left
      if (newCursor < cursor) {
	tx1 = cursorToX(newCursor);
	tx2 = cursorToX(cursor);
	XFillRectangle(getDisplay(), xwin, getXorGC(),
		       tx1, ty, tx2 - tx1, textHeight);
      }

      // grew on the right
      if (newSelectionEnd > selectionEnd) {
	tx1 = cursorToX(selectionEnd);
	tx2 = cursorToX(newSelectionEnd);
	XFillRectangle(getDisplay(), xwin, getXorGC(),
		       tx1, ty, tx2 - tx1, textHeight);
      }

      cursor = newCursor;
      selectionEnd = newSelectionEnd;
    }
  }
}

void LTKTextIn::redrawTail(int i) {
  int tx, ty;
  int j;

  // make sure cursor is visible
  if (cursor < firstChar) {
    firstChar = cursor;
    i = firstChar;
  } else if (cursor > firstChar) {
    tx = cursorToX(cursor);
    if (tx > width - horizBorder - 1) {
      do {
	++firstChar;
	tx = cursorToX(cursor);
      } while (tx > width - horizBorder - 1 && firstChar < cursor);
      i = firstChar;
    }
  }

  // redraw if necessary
  if (i >= 0) {
    tx = cursorToX(i);
    ty = (height - textHeight) / 2;
    XFillRectangle(getDisplay(), xwin, getBgGC(), tx, 0, width - tx, height);
    ty += textBase;
    j = xToCursor(width);
    if (j < text->getLength())
      ++j;
    XDrawString(getDisplay(), xwin, textGC, tx, ty,
		text->getCString() + i, j - i);
  }
}
