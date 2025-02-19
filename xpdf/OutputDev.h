//========================================================================
//
// OutputDev.h
//
//========================================================================

#ifndef OUTPUTDEV_H
#define OUTPUTDEV_H

#pragma interface

#include <stypes.h>

class GfxState;
class GfxColorSpace;
class Stream;

//------------------------------------------------------------------------
// OutputDev
//------------------------------------------------------------------------

class OutputDev {
public:

  // Constructor.
  OutputDev() {}

  // Destructor.
  virtual ~OutputDev() {}

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual Boolean upsideDown() { return false; }

  // Set page size (in pixels).
  virtual void setPageSize(int x, int y) {}

  // Reset state and clear display, to prepare for a new page.
  virtual void clear() {}

  // Dump page contents to display.
  virtual void dump() {}

  //----- save/restore graphics state
  virtual void saveState(GfxState *state) {}
  virtual void restoreState(GfxState *state) {}

  //----- update graphics state
  virtual void updateAll(GfxState *state) {}
  virtual void updateCTM(GfxState *state) {}
  virtual void updateLineDash(GfxState *state) {}
  virtual void updateFlatness(GfxState *state) {}
  virtual void updateLineJoin(GfxState *state) {}
  virtual void updateLineCap(GfxState *state) {}
  virtual void updateMiterLimit(GfxState *state) {}
  virtual void updateLineWidth(GfxState *state) {}
  virtual void updateFillColor(GfxState *state) {}
  virtual void updateStrokeColor(GfxState *state) {}
  virtual void updateFont(GfxState *state) {}

  //----- path painting
  virtual void stroke(GfxState *state) = 0;
  virtual void fill(GfxState *state) = 0;
  virtual void eoFill(GfxState *state) = 0;

  //----- path clipping
  virtual void clip(GfxState *state) {}
  virtual void eoClip(GfxState *state) {}

  //----- text drawing
  virtual void drawChar(GfxState *state, double x, double y,
			ushort c) = 0;

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Stream *str,
			     int width, int height, Boolean invert) = 0;
  virtual void drawImage(GfxState *state, Stream *str, int width,
			 int height, GfxColorSpace *colorSpace) = 0;
};

#endif
