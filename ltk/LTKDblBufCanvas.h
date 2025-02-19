//========================================================================
//
// LTKDblBufCanvas.h
//
//========================================================================

#ifndef LTKDBLBUFCANVAS_H
#define LTKDBLBUFCANVAS_H

#pragma interface

#include <X11/Xlib.h>
#include <stypes.h>
#include <LTKWidget.h>

class LTKDblBufCanvas: public LTKWidget {
public:

  LTKDblBufCanvas(char *name1, int minWidth1, int minHeight1);

  virtual ~LTKDblBufCanvas();

  virtual LTKWidget *copy() { return new LTKDblBufCanvas(this); }

  virtual long getEventMask();

  Pixmap getPixmap() { return pixmap; }

  virtual void layout1();

  virtual void layout3();

  virtual void redraw();

protected:

  LTKDblBufCanvas(LTKDblBufCanvas *canvas);

  int minWidth, minHeight;	// minimum size

  Pixmap pixmap;		// the off-screen drawable
};

#endif
