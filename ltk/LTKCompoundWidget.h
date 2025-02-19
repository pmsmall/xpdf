//========================================================================
//
// LTKCompoundWidget.h
//
// Compound widget base class.
//
// Copyright 1997-2002 Glyph & Cog, LLC
//
//========================================================================

#ifndef LTKCOMPOUNDWIDGET_H
#define LTKCOMPOUNDWIDGET_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include "gtypes.h"
#include "LTKWidget.h"

//------------------------------------------------------------------------
// LTKCompoundWidget
//------------------------------------------------------------------------

class LTKCompoundWidget: public LTKWidget {
public:

  //---------- constructor and destructor ----------

  LTKCompoundWidget(char *nameA, int widgetNumA);
  virtual ~LTKCompoundWidget();

  //---------- access ----------

  virtual void setParent(LTKWindow *parentA);

  //---------- layout ----------

  virtual void layout1();
  virtual void layout2(int xA, int yA, int widthA, int heightA);
  virtual void layout3();
  virtual void map();

  //---------- drawing ----------

  virtual void redraw();
  virtual void redrawBackground();

protected:

  LTKBox *box;
};

#endif
