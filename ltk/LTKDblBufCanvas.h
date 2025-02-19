//========================================================================
//
// LTKDblBufCanvas.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKDBLBUFCANVAS_H
#define LTKDBLBUFCANVAS_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include <LTKWidget.h>

//------------------------------------------------------------------------
// LTKDblBufCanvas
//------------------------------------------------------------------------

class LTKDblBufCanvas: public LTKWidget {
public:

  //---------- constructors and destructor ----------

  LTKDblBufCanvas(char *name1, int widgetNum1,
		  int minWidth1, int minHeight1);

  virtual ~LTKDblBufCanvas();

  virtual LTKWidget *copy() { return new LTKDblBufCanvas(this); }

  //---------- special access ----------

  Pixmap getPixmap() { return pixmap; }

  //---------- layout ----------

  virtual void layout1();
  virtual void layout3();

  //---------- drawing ----------

  virtual void redraw();

protected:

  LTKDblBufCanvas(LTKDblBufCanvas *canvas);

  int minWidth, minHeight;	// minimum size
  Pixmap pixmap;		// the off-screen drawable
};

#endif
