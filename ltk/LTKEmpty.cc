//========================================================================
//
// LTKEmpty.cc
//
//========================================================================

#pragma implementation

#include <stdlib.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <LTKWindow.h>
#include <LTKEmpty.h>

LTKEmpty::LTKEmpty():
    LTKWidget(ltkEmpty, NULL) {
}

LTKEmpty::~LTKEmpty() {
}

void LTKEmpty::layout1() {
  width = 1;
  height = 1;
}

void LTKEmpty::redraw() {
}
