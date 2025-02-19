//========================================================================
//
// LTKCanvas.h
//
// Copyright 1996 Derek B. Noonburg
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

  LTKCanvas(char *name1, int widgetNum1,
	    int minWidth1, int minHeight1, LTKRedrawCbk redrawCbk1);

  //---------- layout ----------

  virtual void layout1();

  //---------- drawing ----------

  virtual void redraw();

protected:

  int minWidth, minHeight;	// minimum size

  LTKRedrawCbk redrawCbk;	// redraw callback
};

#endif
