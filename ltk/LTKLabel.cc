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

#include <aconf.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "LTKLabel.h"

LTKLabel::LTKLabel(char *nameA, int widgetNumA,
		   LTKLabelSize sizeA, int maxLengthA,
		   char *fontNameA, char *textA):
    LTKWidget(nameA, widgetNumA) {
  size = sizeA;
  if (size == ltkLabelMaxLength)
    maxLength = maxLengthA;
  else
    maxLength = -1;
  text = textA ? new GString(textA) : new GString();
  length = text->getLength();
  if (maxLength >= 0 && length > maxLength)
    length = maxLength;
  fontName = fontNameA;
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

void LTKLabel::setText(char *textA) {
  if (size == ltkLabelStatic)
    return;
  delete text;
  text = textA ? new GString(textA) : new GString();
  length = text->getLength();
  if (maxLength >= 0 && length > maxLength)
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
  int textWidth;

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
  textWidth = 0;
  switch (size) {
  case ltkLabelStatic:
    XTextExtents(fontStruct, text->getCString(), text->getLength(),
		 &direction, &ascent, &descent, &extents);
    textWidth = extents.width;
    break;
  case ltkLabelFixedWidth:
    textWidth = 0;
    break;
  case ltkLabelMaxLength:
    textWidth = maxLength * fontStruct->max_bounds.width;
    break;
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
