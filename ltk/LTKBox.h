//========================================================================
//
// LTKBox.h
//
//========================================================================

#ifndef LTKBOX_H
#define LTKBOX_H

#pragma interface

#include <X11/Xlib.h>
#include <stypes.h>
#include <LTKBorder.h>
#include <LTKWidget.h>

class LTKBox: public LTKWidget {
public:

  LTKBox(char *name1, int cols1, int rows1,
	 int left1, int right1, int top1, int bottom1,
	 LTKBorder border1, int xfill1, int yfill1, ...);

  ~LTKBox();

  virtual LTKWidget *copy() { return new LTKBox(this); }

  virtual void setParent(LTKWindow *parent1);
  int getXFill() { return xfill; }
  int getYFill() { return yfill; }

  Boolean checkFills(char **err);

  virtual void layout1();
  virtual void layout2(int x1, int y1, int width1, int height1);
  virtual void layout3();

  virtual void redraw();
  virtual void map();

  void setBorder(LTKBorder border1);

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
