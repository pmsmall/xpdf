//========================================================================
//
// LTKTextIn.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#pragma implementation

#include <stdlib.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <LTKWindow.h>
#include <LTKTextIn.h>
#include <LTKBorder.h>

LTKTextIn::LTKTextIn(char *name1, int maxLength1, char *fontName1,
		     LTKTextInCbk cbk1, int widgetNum1):
    LTKWidget(ltkTextIn, name1) {
  maxLength = maxLength1;
  text = new String();
  active = false;
  cursor = 0;
  cbk = cbk1;
  widgetNum = widgetNum1;
  fontName = fontName1;
  fontStruct = NULL;
  textGC = None;
}

LTKTextIn::LTKTextIn(LTKTextIn *textIn):
    LTKWidget(textIn) {
  maxLength = textIn->maxLength;
  text = new String();
  active = false;
  cursor = 0;
  cbk = textIn->cbk;
  widgetNum = textIn->widgetNum;
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
  return ExposureMask | ButtonPressMask | KeyPressMask;
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
  textWidth = maxLength * fontStruct->max_bounds.width;
  textHeight = fontStruct->ascent + fontStruct->descent;
  textBase = fontStruct->ascent;
  width = textWidth + 12;
  height = textHeight + 4;
}

void LTKTextIn::redraw() {
  int tx, ty;

  tx = 6;
  ty = (height - textHeight) / 2 + textBase;
  XDrawString(getDisplay(), xwin, textGC, tx, ty,
	      text->getCString(), text->getLength());
  drawCursor(active);
}

void LTKTextIn::buttonPress(int mx, int my, int button) {
  XCharStruct extents;
  int direction, ascent, descent;
  int x1, x2;

  drawCursor(false);
  x2 = 6;
  for (cursor = 0; cursor < text->getLength(); ++cursor) {
    XTextExtents(fontStruct, text->getCString() + cursor, 1,
		 &direction, &ascent, &descent, &extents);
    x1 = x2;
    x2 += extents.width;
    if (mx < (x1 + x2) / 2)
      break;
  }
  drawCursor(true);
}

void LTKTextIn::activate(Boolean on) {
  if (active != on) {
    drawCursor(on);
    parent->setKeyWidget(on ? this : (LTKWidget *)NULL);
    active = on;
    if (!active && cbk)
      (*cbk)(this, widgetNum, text);
  }
}

void LTKTextIn::keyPress(KeySym key, char *s, int n) {
  if (n >= 1 && (s[0] == '\n' || s[0] == '\r')) {
    activate(false);
    return;
  }

  drawCursor(false);

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
      redrawTail(0);
      break;
    case '\b':			// bs
    case '\177':		// del
      if (cursor > 0) {
	text->del(cursor - 1);
	--cursor;
	redrawTail(cursor);
      }
      break;
    case '\004':		// ^D
      if (cursor < text->getLength()) {
	text->del(cursor);
	redrawTail(cursor);
      }
      break;
    case '\013':		// ^K
      text->del(cursor, text->getLength() - cursor);
      redrawTail(cursor);
      break;
    case '\025':		// ^U
      text->clear();
      cursor = 0;
      redrawTail(cursor);
      break;
    default:
      if (s[0] >= 0x20) {
	if (text->getLength() < maxLength) {
	  text->insert(cursor, s);
	  redrawTail(cursor);
	  ++cursor;
	}
      }
      break;
    }
  }

  drawCursor(true);
}

void LTKTextIn::drawCursor(Boolean on) {
  XCharStruct extents;
  int direction, ascent, descent;
  int tx, ty;

  tx = 6;
  ty = (height - textHeight) / 2;
  if (cursor > 0) {
    XTextExtents(fontStruct, text->getCString(), cursor,
		 &direction, &ascent, &descent, &extents);
    tx += extents.width;
  }
  if (on) {
    XDrawLine(getDisplay(), xwin, textGC, tx-1, ty, tx-1, ty + textHeight);
    XDrawLine(getDisplay(), xwin, textGC, tx, ty, tx, ty + textHeight);
  } else {
    XDrawLine(getDisplay(), xwin, getBgGC(), tx-1, ty, tx-1, ty + textHeight);
    XDrawLine(getDisplay(), xwin, getBgGC(), tx, ty, tx, ty + textHeight);
    if (cursor < text->getLength()) {
      ty += textBase;
      XDrawString(getDisplay(), xwin, textGC, tx, ty,
		  text->getCString() + cursor, 1);
    }
  }
}

void LTKTextIn::redrawTail(int i) {
  XCharStruct extents;
  int direction, ascent, descent;
  int tx, ty;

  tx = 6;
  ty = (height - textHeight) / 2;
  if (i > 0) {
    XTextExtents(fontStruct, text->getCString(), i,
		 &direction, &ascent, &descent, &extents);
    tx += extents.width;
  }
  XFillRectangle(getDisplay(), xwin, getBgGC(), tx, 0, width - tx, height);
  ty += textBase;
  XDrawString(getDisplay(), xwin, textGC, tx, ty,
	      text->getCString() + i, text->getLength() - i);
}

void LTKTextIn::setText(char *s) {
  delete text;
  if (strlen(s) <= (uint)maxLength)
    text = new String(s);
  else
    text = new String(s, maxLength);
  if (getXWindow() != None)
    redrawTail(0);
}
