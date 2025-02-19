//========================================================================
//
// LTKEmpty.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTK_H
#define LTK_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include <LTKWidget.h>

//------------------------------------------------------------------------
// LTKEmpty
//------------------------------------------------------------------------

class LTKEmpty: public LTKWidget {
public:

  //---------- constructors and destructor ----------

  LTKEmpty();

  ~LTKEmpty();

  virtual LTKWidget *copy() { return new LTKEmpty(); }

  //---------- layout ----------

  virtual void layout1();

  //---------- drawing ----------

  virtual void redraw();

private:
};

#endif
