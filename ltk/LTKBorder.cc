//========================================================================
//
// LTKBorder.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stddef.h>
#include <gtypes.h>
#include <LTKBorder.h>

Gulong ltkGetBrightColor(Display *display, int screenNum,
				XColor *bg, Gulong def) {
  XColor bright;
  Gulong ret;
  Gulong t;

  if ((bg->red == 0 && bg->green == 0 && bg->blue == 0) ||
      (bg->red == 65535 && bg->green == 65535 && bg->blue == 65535)) {
    ret = def;
  } else {
    t = (Gulong)bg->red + (Gulong)bg->red / 4;
    bright.red = (t > 65535) ? 65535 : (Gushort)t;
    t = (Gulong)bg->green + (Gulong)bg->green / 4;
    bright.green = (t > 65535) ? 65535 : (Gushort)t;
    t = (Gulong)bg->blue + (Gulong)bg->blue / 4;
    bright.blue = (t > 65535) ? 65535 : (Gushort)t;
    if (XAllocColor(display, DefaultColormap(display, screenNum), &bright))
      ret = bright.pixel;
    else
      ret = def;
  }
  return ret;
}

Gulong ltkGetDarkColor(Display *display, int screenNum,
			      XColor *bg, Gulong def) {
  XColor dark;
  Gulong ret;
  long t;

  if ((bg->red == 0 && bg->green == 0 && bg->blue == 0) ||
      (bg->red == 65535 && bg->green == 65535 && bg->blue == 65535)) {
    ret = def;
  } else {
    t = (Gulong)bg->red - (Gulong)bg->red / 4;
    dark.red = (t < 0) ? 0 : (Gushort)t;
    t = (Gulong)bg->green - (Gulong)bg->green / 4;
    dark.green = (t < 0) ? 0 : (Gushort)t;
    t = (Gulong)bg->blue - (Gulong)bg->blue / 4;
    dark.blue = (t < 0) ? 0 : (Gushort)t;
    if (XAllocColor(display, DefaultColormap(display, screenNum), &dark))
      ret = dark.pixel;
    else
      ret = def;
  }
  return ret;
}

void ltkDrawBorder(Display *display, Window xwin,
		   GC bright, GC dark, GC background,
		   int x, int y, int width, int height,
		   LTKBorder border) {
  GC gc1 = None;
  GC gc2 = None;

  switch (border) {
  case ltkBorderNone:
    gc1 = gc2 = background;
    break;
  case ltkBorderRaised:
    gc1 = bright;
    gc2 = dark;
    break;
  case ltkBorderSunken:
    gc1 = dark;
    gc2 = bright;
    break;
  }
  XDrawLine(display, xwin, gc1, x, y, x+width-1, y);
  XDrawLine(display, xwin, gc1, x+1, y+1, x+width-2, y+1);
  XDrawLine(display, xwin, gc1, x, y, x, y+height-1);
  XDrawLine(display, xwin, gc1, x+1, y+1, x+1, y+height-2);
  XDrawLine(display, xwin, gc2, x, y+height-1, x+width-1, y+height-1);
  XDrawLine(display, xwin, gc2, x+1, y+height-2, x+width-2, y+height-2);
  XDrawLine(display, xwin, gc2, x+width-1, y, x+width-1, y+height-1);
  XDrawLine(display, xwin, gc2, x+width-2, y+1, x+width-2, y+height-2);
}

void ltkDrawTriBorder(Display *display, Window xwin,
		      GC bright, GC dark, GC background,
		      int x, int y, int width, int height,
		      LTKTriangle orient, LTKBorder border) {
  GC gc1 = None;
  GC gc2 = None;
  int x2, y2;

  switch (border) {
  case ltkBorderNone:
    gc1 = gc2 = background;
    break;
  case ltkBorderRaised:
    gc1 = bright;
    gc2 = dark;
    break;
  case ltkBorderSunken:
    gc1 = dark;
    gc2 = bright;
    break;
  }
  switch (orient) {
  case ltkTriLeft:
    y2 = y + (height - 1) / 2;
    XDrawLine(display, xwin, gc1, x+width-1, y, x, y2);
    XDrawLine(display, xwin, gc1, x+width-2, y+1, x+1, y2);
    XDrawLine(display, xwin, gc2, x, y2, x+width-1, y+height-1);
    XDrawLine(display, xwin, gc2, x+1, y2, x+width-2, y+height-2);
    XDrawLine(display, xwin, gc2, x+width-1, y+height-1, x+width-1, y);
    XDrawLine(display, xwin, gc2, x+width-2, y+height-2, x+width-2, y+1);
    break;
  case ltkTriRight:
    y2 = y + (height - 1) / 2;
    XDrawLine(display, xwin, gc1, x+width-1, y2, x, y);
    XDrawLine(display, xwin, gc1, x+width-2, y2, x+1, y+1);
    XDrawLine(display, xwin, gc1, x, y, x, y+height-1);
    XDrawLine(display, xwin, gc1, x+1, y+1, x+1, y+height-2);
    XDrawLine(display, xwin, gc2, x, y+height-1, x+width-1, y2);
    XDrawLine(display, xwin, gc2, x+1, y+height-2, x+width-2, y2);
    break;
  case ltkTriUp:
    x2 = x + (width - 1) / 2;
    XDrawLine(display, xwin, gc1, x2, y, x, y+height-1);
    XDrawLine(display, xwin, gc1, x2, y+1, x+1, y+height-2);
    XDrawLine(display, xwin, gc2, x, y+height-1, x+width-1, y+height-1);
    XDrawLine(display, xwin, gc2, x+1, y+height-2, x+width-2, y+height-2);
    XDrawLine(display, xwin, gc2, x+width-1, y+height-1, x2, y);
    XDrawLine(display, xwin, gc2, x+width-2, y+height-2, x2, y+1);
    break;
  case ltkTriDown:
    x2 = x + (width - 1) / 2;
    XDrawLine(display, xwin, gc1, x+width-1, y, x, y);
    XDrawLine(display, xwin, gc1, x+width-2, y+1, x+1, y+1);
    XDrawLine(display, xwin, gc1, x, y, x2, y+height-1);
    XDrawLine(display, xwin, gc1, x+1, y+1, x2, y+height-2);
    XDrawLine(display, xwin, gc2, x2, y+height-1, x+width-1, y);
    XDrawLine(display, xwin, gc2, x2, y+height-2, x+width-2, y+1);
    break;
  }
}
