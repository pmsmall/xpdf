//========================================================================
//
// XOutputDev.h
//
//========================================================================

#ifndef XOUTPUTDEV_H
#define XOUTPUTDEV_H

#pragma interface

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "config.h"
#include "OutputDev.h"

class LTKApp;
class LTKWindow;
class GfxColor;
class GfxFont;

//------------------------------------------------------------------------
// XOutputFontCache
//------------------------------------------------------------------------

struct XOutputFont {
  char name[100];
  XFontStruct *font;
};

class XOutputFontCache {
public:

  // Constructor.
  XOutputFontCache(Display *display1);

  // Destructor.
  ~XOutputFontCache();

  // Get a font.  This does an XLoadFont if necessary.
  XFontStruct *getFont(char *name);

private:

  Display *display;		// X display pointer
  XOutputFont			// fonts in reverse-LRU order
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
  virtual Boolean upsideDown() { return true; }

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
  virtual void drawChar(GfxState *state, double x, double y, ushort c);

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Stream *str,
			     int width, int height, Boolean invert);
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
  unsigned long			// color cube
    colors[maxColorCube * maxColorCube * maxColorCube];
  int numColors;		// size of color cube
  GfxFont *gFont;		// current font
  XFontStruct *font;		// current X font
  ushort *isoMap;		// font encoding
  ushort *revISOMap;		// reverse encoding
  XOutputFontCache *fontCache;	// font cache
  XOutputState *save;		// stack of saved states

  void updateLineAttrs(GfxState *state, Boolean updateDash);
  unsigned long findColor(GfxColor *color);
  unsigned long findColor(int x[4], GfxColorSpace *colorSpace);
  XPoint *pathPoints(GfxState *state, int *numPoints);
};

#endif
