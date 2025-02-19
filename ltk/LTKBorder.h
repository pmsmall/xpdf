//========================================================================
//
// LTKBorder.h
//
// Copyright 1996-2002 Glyph & Cog, LLC
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

// Compute bright/dark color and allocate X pixel color.
extern Gulong ltkGetBrightColor(Display *display, int screenNum,
				XColor *bg, Gulong def);
extern Gulong ltkGetDarkColor(Display *display, int screenNum,
			      XColor *bg, Gulong def);

// Draw a rectangular border.
extern void ltkDrawBorder(Display *display, Window xwin,
			  GC bright, GC dark, GC background,
			  int x, int y, int width, int height,
			  LTKBorder border);

// Draw a triangular border.
extern void ltkDrawTriBorder(Display *display, Window xwin,
			     GC bright, GC dark, GC background,
			     int x, int y, int width, int height,
			     LTKTriangle orient, LTKBorder border);

// Draw a divider line.  Divider is horizontal if width > 0, vertical
// otherwise.
extern void ltkDrawDivider(Display *display, Window xwin,
			   GC bright, GC dark, GC background,
			   int x, int y, int width, int height,
			   LTKBorder border);

// Draw a border which splits a rectangle into two pieces.
// Border is horizontal if width > 0, vertical otherwise.
// This border has width 2*ltkBorderWidth.
extern void ltkDrawSplitBorder(Display *display, Window xwin,
			       GC bright, GC dark, GC background,
			       int x, int y, int width, int height,
			       LTKBorder border);

#endif
