//========================================================================
//
// LTKCanvas.cc
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
#include "LTKWindow.h"
#include "LTKCanvas.h"

LTKCanvas::LTKCanvas(char *nameA, int widgetNumA,
		     int minWidthA, int minHeightA, LTKRedrawCbk redrawCbkA):
    LTKWidget(nameA, widgetNumA) {
  minWidth = minWidthA;
  minHeight = minHeightA;
  redrawCbk = redrawCbkA;
}

void LTKCanvas::layout1() {
  width = minWidth;
  height = minHeight;
}

void LTKCanvas::redraw() {
  if (redrawCbk)
    (*redrawCbk)(this, widgetNum);
}
