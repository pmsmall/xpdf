//========================================================================
//
// LTKScrollingCanvas.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKSCROLLINGCANVAS_H
#define LTKSCROLLINGCANVAS_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include <LTKWidget.h>

//------------------------------------------------------------------------
// LTKScrollingCanvas
//------------------------------------------------------------------------

class LTKScrollingCanvas: public LTKWidget {
public:

  //---------- constructors and destructor ----------

  LTKScrollingCanvas(char *name1, int widgetNum1,
		     int realWidth1, int realHeight1,
		     int minWidth1, int minHeight1);

  virtual ~LTKScrollingCanvas();

  virtual LTKWidget *copy() { return new LTKScrollingCanvas(this); }

  //---------- special access ----------

  Pixmap getPixmap() { return pixmap; }
  void resize(int realWidth1, int realHeight1);
  int getRealWidth() { return realWidth; }
  int getRealHeight() { return realHeight; }
  int getMaxX() { return realWidth >= width ? realWidth - width : 0; }
  int getMaxY() { return realHeight >= height ? realHeight - height : 0; }
  void scroll(int x, int y);


  //---------- layout ----------

  virtual void layout1();
  virtual void layout3();

  //---------- drawing ----------

  virtual void redraw();

  //---------- callbacks and event handlers ----------

  virtual void buttonPress(int mx, int my, int button);

protected:

  LTKScrollingCanvas(LTKScrollingCanvas *canvas);

  int realWidth, realHeight;	// size of "real" canvas, i.e., backdrop
  int minWidth, minHeight;	// minimum size

  int left, top;		// location of window in "real" canvas
  Pixmap pixmap;		// the "real" canvas drawable
};

#endif
