//========================================================================
//
// LTKCanvas.h
//
//========================================================================

#ifndef LTKCANVAS_H
#define LTKCANVAS_H

#pragma interface

#include <X11/Xlib.h>
#include <stypes.h>
#include <LTKWidget.h>

class LTKCanvas;
typedef void (*LTKCanvasCbk)(LTKCanvas *canvas, int widgetNum);

class LTKCanvas: public LTKWidget {
public:

  LTKCanvas(char *name1, int minWidth1, int minHeight1,
	    LTKCanvasCbk redrawCbk1, int widgetNum1);

  virtual LTKWidget *copy() { return new LTKCanvas(this); }

  virtual long getEventMask();

  virtual void layout1();

  virtual void redraw();

protected:

  LTKCanvas(LTKCanvas *canvas);

  int minWidth, minHeight;	// minimum size
  LTKCanvasCbk redrawCbk;	// redraw callback
  int widgetNum;		// widget number (for callback)
};

#endif
