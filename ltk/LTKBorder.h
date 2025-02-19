//========================================================================
//
// LTKBorder.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKBORDER_H
#define LTKBORDER_H

#pragma interface

#include <X11/Xlib.h>

// Width of a border.
#define ltkBorderWidth 2

// Type of border.
typedef enum {
  ltkBorderNone,
  ltkBorderRaised,
  ltkBorderSunken
} LTKBorder;

// Triangle orientation.
typedef enum {
  ltkTriLeft,
  ltkTriRight,
  ltkTriUp,
  ltkTriDown
} LTKTriangle;

extern unsigned long ltkGetBrightColor(Display *display, int screenNum,
				       XColor *bg, unsigned long def);
extern unsigned long ltkGetDarkColor(Display *display, int screenNum,
				     XColor *bg, unsigned long def);

extern void ltkDrawBorder(Display *display, Window xwin,
			  GC bright, GC dark, GC background,
			  int x, int y, int width, int height,
			  LTKBorder border);

extern void ltkDrawTriBorder(Display *display, Window xwin,
			     GC bright, GC dark, GC background,
			     int x, int y, int width, int height,
			     LTKTriangle orient, LTKBorder border);

#endif
