//========================================================================
//
// OutputDev.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef OUTPUTDEV_H
#define OUTPUTDEV_H

#ifdef __GNUC__
#pragma interface
#endif

#include <gtypes.h>

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
  virtual GBool upsideDown() { return gFalse; }

  // Set page size (in pixels).
  virtual void setPageSize(int x, int y) {}

  // Set transform matrix.
  virtual void setCTM(double *ctm1);

  // Reset state and clear display, to prepare for a new page.
  virtual void clear() {}

  // Dump page contents to display.
  virtual void dump() {}

  // Convert between device and user coordinates.
  virtual void cvtDevToUser(int dx, int dy, double *ux, double *uy);
  virtual void cvtUserToDev(double ux, double uy, int *dx, int *dy);

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
			Guchar c) = 0;

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Stream *str,
			     int width, int height, GBool invert) = 0;
  virtual void drawImage(GfxState *state, Stream *str, int width,
			 int height, GfxColorSpace *colorSpace) = 0;

private:

  double ctm[6];		// coordinate transform matrix
  double ictm[6];		// inverse CTM
};

#endif
