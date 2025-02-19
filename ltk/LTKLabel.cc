//========================================================================
//
// LTKLabel.cc
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
#include <LTKLabel.h>

LTKLabel::LTKLabel(char *name1, int widgetNum1,
		   int maxLength1, char *fontName1, char *text1):
    LTKWidget(name1, widgetNum1) {
  maxLength = maxLength1;
  text = text1 ? new GString(text1) : new GString();
  length = text->getLength();
  if (maxLength >= 0 && length > maxLength)
    length = maxLength;
  fontName = fontName1;
  fontStruct = NULL;
  textGC = None;
}

LTKLabel::LTKLabel(LTKLabel *label):
    LTKWidget(label) {
  maxLength = label->maxLength;
  text = label->text->copy();
  length = label->length;
  fontName = label->fontName;
  fontStruct = NULL;
  textGC = None;
}

LTKLabel::~LTKLabel() {
  delete text;
  if (fontName && fontStruct) {
    XFreeFont(getDisplay(), fontStruct);
    XFreeGC(getDisplay(), textGC);
  }
}

void LTKLabel::setText(char *text1) {
  if (maxLength < 0)
    return;
  delete text;
  text = text1 ? new GString(text1) : new GString();
  length = text->getLength();
  if (length > maxLength)
    length = maxLength;
  if (getXWindow() != None) {
    clear();
    redraw();
  }
}

void LTKLabel::layout1() {
  XCharStruct extents;
  int direction, ascent, descent;
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
  if (maxLength < 0) {
    XTextExtents(fontStruct, text->getCString(), text->getLength(),
		 &direction, &ascent, &descent, &extents);
    textWidth = extents.width;
  } else {
    textWidth = maxLength * fontStruct->max_bounds.width;
  }
  textHeight = fontStruct->ascent + fontStruct->descent;
  textBase = fontStruct->ascent;
  width = textWidth + 12;
  height = textHeight + 4;
}

void LTKLabel::redraw() {
  int tx, ty;

  tx = 6;
  ty = (height - textHeight) / 2 + textBase;
  XDrawString(getDisplay(), xwin, textGC, tx, ty,
	      text->getCString(), length);
}
