//========================================================================
//
// LTKEmpty.h
//
//========================================================================

#ifndef LTK_H
#define LTK_H

#pragma interface

#include <X11/Xlib.h>
#include <stypes.h>
#include <LTKWidget.h>

class LTKEmpty: public LTKWidget {
public:

  LTKEmpty();

  ~LTKEmpty();

  virtual LTKWidget *copy() { return new LTKEmpty(); }

  virtual void layout1();

  virtual void redraw();

private:
};

#endif
