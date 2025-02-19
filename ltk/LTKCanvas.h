//========================================================================
//
// LTKCanvas.h
//
// Copyright 1996-2002 Glyph & Cog, LLC
//
//========================================================================

#ifndef LTKCANVAS_H
#define LTKCANVAS_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include "LTKWidget.h"

//------------------------------------------------------------------------
// LTKCanvas
//------------------------------------------------------------------------

class LTKCanvas: public LTKWidget {
public:

  //---------- constructor ----------

  LTKCanvas(char *nameA, int widgetNumA,
	    int minWidthA, int minHeightA, LTKRedrawCbk redrawCbkA);

  //---------- layout ----------

  virtual void layout1();

  //---------- drawing ----------

  virtual void redraw();

protected:

  int minWidth, minHeight;	// minimum size

  LTKRedrawCbk redrawCbk;	// redraw callback
};

#endif
