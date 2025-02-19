//========================================================================
//
// LTKBorder.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKBORDER_H
#define LTKBORDER_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>

// Width of a border.
#define ltkBorderWidth 2

// Type of border.
enum LTKBorder {
  ltkBorderNone,
  ltkBorderRaised,
  ltkBorderSunken
};

// Triangle orientation.
enum LTKTriangle {
  ltkTriLeft,
  ltkTriRight,
  ltkTriUp,
  ltkTriDown
};

extern Gulong ltkGetBrightColor(Display *display, int screenNum,
				XColor *bg, Gulong def);
extern Gulong ltkGetDarkColor(Display *display, int screenNum,
			      XColor *bg, Gulong def);

extern void ltkDrawBorder(Display *display, Window xwin,
			  GC bright, GC dark, GC background,
			  int x, int y, int width, int height,
			  LTKBorder border);

extern void ltkDrawTriBorder(Display *display, Window xwin,
			     GC bright, GC dark, GC background,
			     int x, int y, int width, int height,
			     LTKTriangle orient, LTKBorder border);

#endif
