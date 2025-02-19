//========================================================================
//
// LTKList.cc
//
// Copyright 1997 Derek B. Noonburg
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
#include "gmem.h"
#include "LTKList.h"

#define horizBorder 4
#define vertBorder  2

LTKList::LTKList(char *name1, int widgetNum1,
		 int minWidth1, int minLines1,
		 GBool allowSelection1, char *fontName1):
    LTKWidget(name1, widgetNum1) {
  minWidth = minWidth1;
  minLines = minLines1;
  allowSelection = allowSelection1;
  clickCbk = NULL;
  dblClickCbk = NULL;
  text = NULL;
  numLines = 0;
  textSize = 0;
  topLine = 0;
  horizOffset = 0;
  selection = -1;
  fontName = fontName1;
  fontStruct = NULL;
  textGC = None;
}

LTKList::~LTKList() {
  int i;

  for (i = 0; i < numLines; ++i)
    delete text[i];
  gfree(text);
  if (fontName && fontStruct) {
    XFreeFont(getDisplay(), fontStruct);
    XFreeGC(getDisplay(), textGC);
  }
}

long LTKList::getEventMask() {
  return LTKWidget::getEventMask() | ButtonPressMask | ButtonReleaseMask;
}

void LTKList::insertLine(int line, char *s) {
  XCharStruct extents;
  int direction, ascent, descent;
  int i;

  if (line <= numLines) {
    if (numLines >= textSize) {
      textSize += 32;
      text = (GString **)grealloc(text, textSize * sizeof(GString *));
    }
    for (i = numLines-1; i >= line; --i)
      text[i+1] = text[i];
    text[line] = new GString(s);
    ++numLines;
    if (getXWindow() != None) {
      XTextExtents(fontStruct, text[line]->getCString(),
		   text[line]->getLength(),
		   &direction, &ascent, &descent, &extents);
      if (extents.width + 2*horizBorder > maxWidth)
	maxWidth = extents.width + 2*horizBorder;
      redrawBelow(line);
    }
  }
}

void LTKList::replaceLine(int line, char *s) {
  XCharStruct extents;
  int direction, ascent, descent;

  if (line < numLines) {
    delete text[line];
    text[line] = new GString(s);
    if (getXWindow() != None) {
      XTextExtents(fontStruct, text[line]->getCString(),
		   text[line]->getLength(),
		   &direction, &ascent, &descent, &extents);
      if (extents.width + 2*horizBorder > maxWidth)
	maxWidth = extents.width + 2*horizBorder;
      redrawLine(line);
    }
  }
}

//~ this should reduce size of text array
void LTKList::deleteLine(int line) {
  int i;

  if (line < numLines) {
    delete text[line];
    for (i = line; i < numLines - 1; ++i)
      text[i] = text[i+1];
    text[numLines-1] = NULL;
    --numLines;
    if (selection == line) {
      xorSelection();
      selection = -1;
    }
    if (getXWindow() != None)
      redrawBelow(line);
  }
}

void LTKList::deleteAll() {
  int line;

  for (line = 0; line < numLines; ++line)
    delete text[line];
  delete text;
  text = NULL;
  numLines = 0;
  textSize = 0;
  topLine = 0;
  horizOffset = 0;
  selection = -1;
  if (getXWindow() != None) {
    maxWidth = 0;
    redraw();
  }
}

void LTKList::setSelection(int line) {
  if (allowSelection) {
    xorSelection();
    selection = line;
    xorSelection();
  }
}

int LTKList::getDisplayedLines() {
  return (height - 2 * vertBorder) / textHeight;
}

void LTKList::scrollTo(int line, int horiz) {
  GBool changed;

  changed = gFalse;
  if (line != topLine && line < numLines) {
    topLine = line;
    changed = gTrue;
  }
  if (horiz != horizOffset) {
    horizOffset = horiz;
    changed = gTrue;
  }
  //~ blit-scroll if possible
  if (changed && getXWindow() != None)
    redraw();
}

void LTKList::makeVisible(int line) {
  int dispLines;

  dispLines = getDisplayedLines();
  if (line < topLine)
    scrollTo(line, horizOffset);
  else if (line >= topLine + dispLines)
    scrollTo(line - dispLines + 1, horizOffset);
}

void LTKList::layout1() {
  XGCValues gcValues;
  XCharStruct extents;
  int direction, ascent, descent;
  int line;

  // get/create GCs
  if (textGC == None) {
    if (fontName &&
	(fontStruct = XLoadQueryFont(getDisplay(), fontName))) {
      XGetGCValues(getDisplay(), getFgGC(),
		   GCForeground | GCBackground | GCGraphicsExposures,
		   &gcValues);
      gcValues.font = fontStruct->fid;
      textGC = XCreateGC(getDisplay(), parent->getXWindow(),
			 GCForeground | GCBackground | GCGraphicsExposures |
			 GCFont,
			 &gcValues);
    } else {
      fontName = NULL;
      fontStruct = parent->getXFontStruct();
      textGC = getFgGC();
    }
  }

  // compute max line width
  maxWidth = 0;
  for (line = 0; line < numLines; ++line) {
    XTextExtents(fontStruct, text[line]->getCString(), text[line]->getLength(),
		 &direction, &ascent, &descent, &extents);
    if (extents.width > maxWidth)
      maxWidth = extents.width;
  }
  maxWidth += 2 * horizBorder;

  // text parameters
  textHeight = fontStruct->ascent + fontStruct->descent;
  textBase = fontStruct->ascent;

  // compute min width/height
  width = minWidth + 2 * horizBorder;
  height = minLines * textHeight + 2 * vertBorder;
}

void LTKList::layout3() {
  XRectangle rect;

  LTKWidget::layout3();
  rect.x = horizBorder;
  rect.y = vertBorder;
  rect.width = width - 2 * horizBorder;
  rect.height = height - 2 * vertBorder;
  XSetClipRectangles(getDisplay(), textGC, 0, 0, &rect, 1, Unsorted);
}

void LTKList::redraw() {
  int y;
  int i;

  XFillRectangle(getDisplay(), xwin, getBgGC(), 0, 0, width, height);
  y = vertBorder;
  for (i = topLine;
       i < numLines && y < height - vertBorder;
       ++i, y += textHeight) {
    XDrawString(getDisplay(), xwin, textGC,
		horizBorder - horizOffset, y + textBase,
		text[i]->getCString(), text[i]->getLength());
  }
  xorSelection();
}

void LTKList::redrawLine(int line) {
  int y;

  if (line < topLine ||
      line >= topLine +
              (height - 2*vertBorder + textHeight - 1) / textHeight)
    return;
  y = vertBorder + (line - topLine) * textHeight;
  XFillRectangle(getDisplay(), xwin, getBgGC(),
		 horizBorder, y, width - 2*horizBorder, textHeight);
  XDrawString(getDisplay(), xwin, textGC,
	      horizBorder - horizOffset, y + textBase,
	      text[line]->getCString(), text[line]->getLength());
  if (selection == line)
    xorSelection();
}

void LTKList::redrawBelow(int line) {
  int y;
  int i;

  if (line >= topLine +
              (height - 2*vertBorder + textHeight - 1) / textHeight)
    return;
  y = vertBorder + (line - topLine) * textHeight;
  XFillRectangle(getDisplay(), xwin, getBgGC(),
		 horizBorder, y,
		 width - 2*horizBorder, height - vertBorder - y);
  for (i = line;
       i < numLines && y < height - vertBorder;
       ++i, y += textHeight) {
    XDrawString(getDisplay(), xwin, textGC,
		horizBorder - horizOffset, y + textBase,
		text[i]->getCString(), text[i]->getLength());
  }
  if (selection >= line)
    xorSelection();
}

void LTKList::xorSelection() {
  if (selection < topLine ||
      selection >= topLine +
                   (height - 2*vertBorder + textHeight - 1) / textHeight)
    return;
  y = vertBorder + (selection - topLine) * textHeight;
  XFillRectangle(getDisplay(), xwin, getXorGC(),
		 horizBorder, y, width - 2*horizBorder, textHeight);
}

void LTKList::buttonPress(int mx, int my, int button, GBool dblClick) {
  int line;

  line = topLine + (my - vertBorder) / textHeight;
  if (line < numLines) {
    setSelection(line);
    if (dblClick) {
      if (dblClickCbk)
	(*dblClickCbk)(this, widgetNum, selection);
    } else {
      if (clickCbk)
	(*clickCbk)(this, widgetNum, selection);
    }
  }
}
