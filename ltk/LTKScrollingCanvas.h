//========================================================================
//
// LTKScrollingCanvas.h
//
//========================================================================

#ifndef LTKSCROLLINGCANVAS_H
#define LTKSCROLLINGCANVAS_H

#pragma interface

#include <X11/Xlib.h>
#include <stypes.h>
#include <LTKWidget.h>

class LTKScrollingCanvas;
typedef void (*LTKScrollingCanvasCbk)(LTKScrollingCanvas *canvas);

class LTKScrollingCanvas: public LTKWidget {
public:

  LTKScrollingCanvas(char *name1, int realWidth1, int realHeight1,
		     int minWidth1, int minHeight1,
		     LTKScrollingCanvasCbk layoutCbk1);

  virtual ~LTKScrollingCanvas();

  virtual LTKWidget *copy() { return new LTKScrollingCanvas(this); }

  virtual long getEventMask();

  Pixmap getPixmap() { return pixmap; }

  virtual void layout1();

  virtual void layout3();

  virtual void redraw();

  void resize(int realWidth1, int realHeight1);

  int getRealWidth() { return realWidth; }
  int getRealHeight() { return realHeight; }

  int getMaxX() { return realWidth >= width ? realWidth - width : 0; }
  int getMaxY() { return realHeight >= height ? realHeight - height : 0; }

  void scroll(int x, int y);

protected:

  LTKScrollingCanvas(LTKScrollingCanvas *canvas);

  int realWidth, realHeight;	// size of "real" canvas, i.e., backdrop
  int minWidth, minHeight;	// minimum size
  LTKScrollingCanvasCbk		// layout-change callback
    layoutCbk;

  int left, top;		// location of window in "real" canvas
  Pixmap pixmap;		// the "real" canvas drawable
};

#endif
