//========================================================================
//
// LTKCanvas.cc
//
//========================================================================

#pragma implementation

#include <stdlib.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <LTKWindow.h>
#include <LTKCanvas.h>

LTKCanvas::LTKCanvas(char *name1, int minWidth1, int minHeight1,
		     LTKCanvasCbk redrawCbk1, int widgetNum1):
    LTKWidget(ltkCanvas, name1) {
  minWidth = minWidth1;
  minHeight = minHeight1;
  redrawCbk = redrawCbk1;
  widgetNum = widgetNum1;
}

LTKCanvas::LTKCanvas(LTKCanvas *canvas):
    LTKWidget(canvas) {
  minWidth = canvas->minWidth;
  minHeight = canvas->minHeight;
  redrawCbk = canvas->redrawCbk;
  widgetNum = canvas->widgetNum;
}

long LTKCanvas::getEventMask() {
  return ExposureMask;
}

void LTKCanvas::layout1() {
  width = minWidth;
  height = minHeight;
}

void LTKCanvas::redraw() {
  if (redrawCbk)
    (*redrawCbk)(this, widgetNum);
}
