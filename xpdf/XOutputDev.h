//========================================================================
//
// XOutputDev.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef XOUTPUTDEV_H
#define XOUTPUTDEV_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "config.h"
#include "OutputDev.h"

class GString;
class LTKApp;
class LTKWindow;
class GfxColor;
class GfxFont;
struct RGBColor;

#define maxRGBCube 8		// max size of RGB color cube

#define numTmpPoints 256	// number of XPoints in temporary array

//------------------------------------------------------------------------
// XOutputFont
//------------------------------------------------------------------------

class XOutputFont {
public:

  // Constructor.
  XOutputFont(GfxFont *gfxFont, double m11, double m12,
	      double m21, double m22, Display *display1);

  // Destructor.
  ~XOutputFont();

  // Does this font match the tag, size, and angle?
  GBool matches(GString *tag1, double m11, double m12, double m21, double m22)
    { return tag->cmp(tag1) == 0 && mat11 == m11 && mat12 == m12 &&
	     mat21 == m21 && mat22 == m22; }

  // Get X font.
  XFontStruct *getXFont() { return xFont; }

  // Get character mapping.
  Gushort mapChar(Guchar c) { return map[c]; }

  // Reverse map a character.
  Guchar revMapChar(Gushort c) { return revMap[c]; }

private:

  GString *tag;
  double mat11, mat12, mat21, mat22;
  Display *display;
  XFontStruct *xFont;
  Gushort map[256];
  Guchar *revMap;
};

//------------------------------------------------------------------------
// XOutputFontCache
//------------------------------------------------------------------------

class XOutputFontCache {
public:

  // Constructor.
  XOutputFontCache(Display *display1);

  // Destructor.
  ~XOutputFontCache();

  // Get a font.  This creates a new font if necessary.
  XOutputFont *getFont(GfxFont *gfxFont, double m11, double m12,
		       double m21, double m22);

private:

  Display *display;		// X display pointer
  XOutputFont *			// fonts in reverse-LRU order
    fonts[fontCacheSize];
  int numFonts;			// number of valid entries
};

//------------------------------------------------------------------------
// XOutputState
//------------------------------------------------------------------------

struct XOutputState {
  GC strokeGC;
  GC fillGC;
  Region clipRegion;
  XOutputState *next;
};

//------------------------------------------------------------------------
// XOutputDev
//------------------------------------------------------------------------

class XOutputDev: public OutputDev {
public:

  // Constructor.
  XOutputDev(LTKWindow *win1);

  // Destructor.
  virtual ~XOutputDev();

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() { return gTrue; }

  // Set page size (in pixels).
  virtual void setPageSize(int x, int y);

  // Reset state and clear display, to prepare for a new page.
  virtual void clear();

  // Dump page contents to display.
  virtual void dump();

  //----- save/restore graphics state
  virtual void saveState(GfxState *state);
  virtual void restoreState(GfxState *state);

  //----- update graphics state
  virtual void updateAll(GfxState *state);
  virtual void updateCTM(GfxState *state);
  virtual void updateLineDash(GfxState *state);
  virtual void updateLineJoin(GfxState *state);
  virtual void updateLineCap(GfxState *state);
  virtual void updateMiterLimit(GfxState *state);
  virtual void updateLineWidth(GfxState *state);
  virtual void updateFillColor(GfxState *state);
  virtual void updateStrokeColor(GfxState *state);
  virtual void updateFont(GfxState *state);

  //----- path painting
  virtual void stroke(GfxState *state);
  virtual void fill(GfxState *state);
  virtual void eoFill(GfxState *state);

  //----- path clipping
  virtual void clip(GfxState *state);
  virtual void eoClip(GfxState *state);

  //----- text drawing
  virtual void drawChar(GfxState *state, double x, double y, Guchar c);

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Stream *str,
			     int width, int height, GBool invert);
  virtual void drawImage(GfxState *state, Stream *str, int width,
			 int height, GfxColorSpace *colorSpace);

private:

  LTKWindow *win;		// window
  LTKScrollingCanvas *canvas;	// drawing canvas
  Display *display;		// X display pointer
  int screenNum;		// X screen number
  Pixmap pixmap;		// pixmap to draw into
  GC paperGC;			// GC for background
  GC strokeGC;			// GC with stroke color
  GC fillGC;			// GC with fill color
  Region clipRegion;		// clipping region
  Gulong			// color cube
    colors[maxRGBCube * maxRGBCube * maxRGBCube];
  int numColors;		// size of color cube
  XPoint			// temporary points array
    tmpPoints[numTmpPoints];
  GfxFont *gfxFont;		// current PDF font
  XOutputFont *font;		// current font
  XOutputFontCache *fontCache;	// font cache
  XOutputState *save;		// stack of saved states

  void updateLineAttrs(GfxState *state, GBool updateDash);
  void doFill(GfxState *state, int rule);
  XPoint *pathPoints(GfxState *state, int *numPoints);
  Gulong findColor(GfxColor *color);
  Gulong findColor(RGBColor *x, RGBColor *err);
};

#endif
