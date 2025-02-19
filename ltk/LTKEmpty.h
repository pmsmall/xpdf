//========================================================================
//
// LTKEmpty.h
//
// Copyright 1996-2002 Glyph & Cog, LLC
//
//========================================================================

#ifndef LTK_H
#define LTK_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include "LTKWidget.h"

//------------------------------------------------------------------------
// LTKEmpty
//------------------------------------------------------------------------

class LTKEmpty: public LTKWidget {
public:

  //---------- constructor ----------

  LTKEmpty();

  //---------- layout ----------

  virtual void layout1();

  //---------- drawing ----------

  virtual void redraw();

private:
};

#endif
