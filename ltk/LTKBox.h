//========================================================================
//
// LTKBox.h
//
// Copyright 1996-2002 Glyph & Cog, LLC
//
//========================================================================

#ifndef LTKBOX_H
#define LTKBOX_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include "gtypes.h"
#include "LTKBorder.h"
#include "LTKWidget.h"

//------------------------------------------------------------------------
// LTKBox
//------------------------------------------------------------------------

class LTKBox: public LTKWidget {
public:

  //---------- constructor and destructor ----------

  LTKBox(char *nameA, int colsA, int rowsA,
	 int leftA, int rightA, int topA, int bottomA,
	 LTKBorder borderA, int xfillA, int yfillA, ...);

  virtual ~LTKBox();

  //---------- access ----------

  virtual GBool isBox() { return gTrue; }
  virtual void setParent(LTKWindow *parentA);
  virtual void setCompoundParent(LTKWidget *compParentA);
  int getXFill() { return xfill; }
  int getYFill() { return yfill; }

  //---------- special access ----------

  void setBorder(LTKBorder borderA);

  //---------- layout ----------

  GBool checkFills(char **err);
  virtual void layout1();
  virtual void layout2(int xA, int yA, int widthA, int heightA);
  virtual void layout3();
  virtual void map();

  //---------- drawing ----------

  virtual void redraw();
  virtual void redrawBackground();

protected:

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
