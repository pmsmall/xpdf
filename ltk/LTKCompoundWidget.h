//========================================================================
//
// LTKCompoundWidget.h
//
// Compound widget base class.
//
// Copyright 1997 Derek B. Noonburg
//
//========================================================================

#ifndef LTKCOMPOUNDWIDGET_H
#define LTKCOMPOUNDWIDGET_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include <gtypes.h>
#include <LTKWidget.h>

//------------------------------------------------------------------------
// LTKCompoundWidget
//------------------------------------------------------------------------

class LTKCompoundWidget: public LTKWidget {
public:

  //---------- constructor and destructor ----------

  LTKCompoundWidget(char *name1, int widgetNum1);
  virtual ~LTKCompoundWidget();

  //---------- access ----------

  virtual void setParent(LTKWindow *parent1);

  //---------- layout ----------

  virtual void layout1();
  virtual void layout2(int x1, int y1, int width1, int height1);
  virtual void layout3();
  virtual void map();

  //---------- drawing ----------

  virtual void redraw();
  virtual void redrawBackground();

protected:

  LTKBox *box;
};

#endif
