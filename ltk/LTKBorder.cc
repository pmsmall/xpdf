//========================================================================
//
// LTKBorder.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#pragma implementation

#include <LTKBorder.h>

unsigned long ltkGetBrightColor(Display *display, int screenNum,
				XColor *bg, unsigned long def) {
  XColor bright;
  unsigned long ret;

  if ((bg->red == 0 && bg->green == 0 && bg->blue == 0) ||
      (bg->red == 65535 && bg->green == 65535 && bg->blue == 65535)) {
    ret = def;
  } else {
    bright.red = (unsigned short)((2 * (int)bg->red + 65535) / 3);
    bright.green = (unsigned short)((2 * (int)bg->green + 65535) / 3);
    bright.blue = (unsigned short)((2 * (int)bg->blue + 65535) / 3);
    if (XAllocColor(display, DefaultColormap(display, screenNum), &bright))
      ret = bright.pixel;
    else
      ret = def;
  }
  return ret;
}

unsigned long ltkGetDarkColor(Display *display, int screenNum,
			      XColor *bg, unsigned long def) {
  XColor dark;
  unsigned long ret;

  if ((bg->red == 0 && bg->green == 0 && bg->blue == 0) ||
      (bg->red == 65535 && bg->green == 65535 && bg->blue == 65535)) {
    ret = def;
  } else {
    dark.red = (unsigned short)((2 * (int)bg->red) / 3);
    dark.green = (unsigned short)((2 * (int)bg->green) / 3);
    dark.blue = (unsigned short)((2 * (int)bg->blue) / 3);
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
