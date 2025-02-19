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

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "LTKWindow.h"
#include "LTKCanvas.h"

LTKCanvas::LTKCanvas(char *name1, int widgetNum1,
		     int minWidth1, int minHeight1, LTKRedrawCbk redrawCbk1):
    LTKWidget(name1, widgetNum1) {
  minWidth = minWidth1;
  minHeight = minHeight1;
  redrawCbk = redrawCbk1;
}

void LTKCanvas::layout1() {
  width = minWidth;
  height = minHeight;
}

void LTKCanvas::redraw() {
  if (redrawCbk)
    (*redrawCbk)(this, widgetNum);
}
