//========================================================================
//
// LTKDblBufCanvas.h
//
// Copyright 1996-2002 Glyph & Cog, LLC
//
//========================================================================

#ifndef LTKDBLBUFCANVAS_H
#define LTKDBLBUFCANVAS_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include "LTKWidget.h"

//------------------------------------------------------------------------
// LTKDblBufCanvas
//------------------------------------------------------------------------

class LTKDblBufCanvas: public LTKWidget {
public:

  //---------- constructor and destructor ----------

  LTKDblBufCanvas(char *nameA, int widgetNumA,
		  int minWidthA, int minHeightA);

  virtual ~LTKDblBufCanvas();

  //---------- special access ----------

  Pixmap getPixmap() { return pixmap; }

  //---------- layout ----------

  virtual void layout1();
  virtual void layout3();

  //---------- drawing ----------

  virtual void redraw();

protected:

  int minWidth, minHeight;	// minimum size
  Pixmap pixmap;		// the off-screen drawable
};

#endif
