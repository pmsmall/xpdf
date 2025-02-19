//========================================================================
//
// LTKBox.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKBOX_H
#define LTKBOX_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include <gtypes.h>
#include <LTKBorder.h>
#include <LTKWidget.h>

//------------------------------------------------------------------------
// LTKBox
//------------------------------------------------------------------------

class LTKBox: public LTKWidget {
public:

  //---------- constructors and destructor ----------

  LTKBox(char *name1, int cols1, int rows1,
	 int left1, int right1, int top1, int bottom1,
	 LTKBorder border1, int xfill1, int yfill1, ...);

  ~LTKBox();

  virtual LTKWidget *copy() { return new LTKBox(this); }

  //---------- access ----------

  virtual GBool isBox() { return gTrue; }
  virtual void setParent(LTKWindow *parent1);
  int getXFill() { return xfill; }
  int getYFill() { return yfill; }

  //---------- special access ----------

  void setBorder(LTKBorder border1);

  //---------- layout ----------

  GBool checkFills(char **err);
  virtual void layout1();
  virtual void layout2(int x1, int y1, int width1, int height1);
  virtual void layout3();
  virtual void map();

  //---------- drawing ----------

  virtual void redraw();

protected:

  LTKBox(LTKBox *box);
  LTKWidget *&get(int col, int row) { return contents[row*cols + col]; }
  LTKBox *getBox(int col, int row)
    { return (LTKBox *)contents[row*cols + col]; }

  int cols, rows;		// # columns, # rows
  int left, right;		// horizontal padding
  int top, bottom;		// vertical padding
  LTKBorder border;		// no/raised/sunken border
  int borderWidth;		// border size
  int xfill, yfill;		// fill sizes (0 = fit)
  LTKWidget **contents;		// contents, in row-major order
};

#endif
