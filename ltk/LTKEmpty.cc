//========================================================================
//
// LTKEmpty.cc
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
#include "LTKEmpty.h"

LTKEmpty::LTKEmpty():
    LTKWidget(NULL, 0) {
}

void LTKEmpty::layout1() {
  width = 1;
  height = 1;
}

void LTKEmpty::redraw() {
}
