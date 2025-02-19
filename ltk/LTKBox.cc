//========================================================================
//
// LTKBox.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <LTKWindow.h>
#include <LTKBox.h>

LTKBox::LTKBox(char *name1, int cols1, int rows1,
	       int left1, int right1, int top1, int bottom1,
	       LTKBorder border1, int xfill1, int yfill1, ...):
    LTKWidget(name1, 0) {
  int col, row;
  va_list args;

  cols = cols1;
  rows = rows1;
  left = left1;
  right = right1;
  top = top1;
  bottom = bottom1;
  border = border1;
  if (border == ltkBorderNone)
    borderWidth = 0;
  else
    borderWidth = ltkBorderWidth;
  xfill = xfill1;
  yfill = yfill1;
  contents = new LTKWidget*[cols*rows];
  va_start(args, yfill1);
  for (row = 0; row < rows; ++row)
    for (col = 0; col < cols; ++col)
      get(col, row) = va_arg(args, LTKWidget *);
  va_end(args);
}

LTKBox::LTKBox(LTKBox *box):
    LTKWidget(box) {
  int col, row;

  cols = box->cols;
  rows = box->rows;
  left = box->left;
  right = box->right;
  top = box->top;
  bottom = box->bottom;
  border = box->border;
  if (border == ltkBorderNone)
    borderWidth = 0;
  else
    borderWidth = ltkBorderWidth;
  xfill = box->xfill;
  yfill = box->yfill;
  contents = new LTKWidget*[cols*rows];
  for (col = 0; col < cols; ++col)
    for (row = 0; row < rows; ++row)
      get(col, row) = box->get(col, row)->copy();
}

LTKBox::~LTKBox() {
  int col, row;

  for (col = 0; col < cols; ++col)
    for (row = 0; row < rows; ++row)
      delete get(col, row);
  delete[] contents;
}

void LTKBox::setParent(LTKWindow *parent1) {
  int col, row;

  parent = parent1;
  for (col = 0; col < cols; ++col)
    for (row = 0; row < rows; ++row)
      get(col, row)->setParent(parent1);
}

void LTKBox::setBorder(LTKBorder border1) {
  if (border1 != border) {
    border = border1;
    ltkDrawBorder(getDisplay(), getParent()->getXWindow(),
		  getBrightGC(), getDarkGC(), getBgGC(),
		  x, y, width, height, border);
  }
}

GBool LTKBox::checkFills(char **err) {
  LTKWidget *widget;
  LTKBox *box;
  int col, row;

  // all contents of a non-1x1 box must be boxes;
  // non-box widgets don't use fill flags, so just return
  if (cols == 1 && rows == 1) {
    widget = get(0, 0);
    if (!widget->isBox())
      return gTrue;
  } else {
    for (col = 0; col < cols; ++col) {
      for (row = 0; row < rows; ++row) {
	widget = get(col, row);
	if (!widget->isBox()) {
	  *err = "contents of a non-1x1 box must be boxes";
	  return gFalse;
	}
      }
    }
  }

  // all xfills in a given column must be the same;
  // all yfills in a given row must be the same
  if (cols > 1 || rows > 1) {
    for (col = 0; col < cols; ++col) {
      for (row = 0; row < rows; ++row) {
	box = getBox(col, row);
	if (box->xfill != getBox(col, 0)->xfill) {
	  *err = "all xfills in a column must be the same";
	  return gFalse;
	}
	if (box->yfill != getBox(0, row)->yfill) {
	  *err = "all yfills in a row must be the same";
	  return gFalse;
	}
      }
    }
  }

  // check contents
  for (col = 0; col < cols; ++col) {
    for (row = 0; row < rows; ++row) {
      if (!getBox(col, row)->checkFills(err))
	return gFalse;
    }
  }

  // everything checked out ok
  *err = "";
  return gTrue;
}

void LTKBox::layout1() {
  int col, row;
  int dw, dh, w1, h1;
  int t1, t2;

  // do children
  for (col = 0; col < cols; ++col)
    for (row = 0; row < rows; ++row)
      get(col, row)->layout1();

  // if there is only one child and it is not a box, just
  // grab its min width and height
  if (cols == 1 && rows == 1 && !get(0, 0)->isBox()) {
    width = get(0, 0)->getWidth() + left + right + 2 * borderWidth;
    height = get(0, 0)->getHeight() + top + bottom + 2 * borderWidth;
    return;
  }

  // compute min width
  dw = 0;
  for (col = 0; col < cols; ++col)
    dw += getBox(col, 0)->getXFill();
  width = 0;
  w1 = 0;
  for (col = 0; col < cols; ++col) {
    t1 = 0;
    for (row = 0; row < rows; ++row) {
      if (getBox(col, row)->getXFill() > 0)
	t2 = (get(col, row)->getWidth() * dw) / getBox(col, row)->getXFill();
      else
	t2 = get(col, row)->getWidth();
      if (t2 > t1)
	t1 = t2;
    }
    if (getBox(col, 0)->getXFill() > 0) {
      if (t1 > w1)
	w1 = t1;
    } else {
      width += t1;
    }
  }
  width += w1 + left + right + 2 * borderWidth;

  // compute min height
  dh = 0;
  for (row = 0; row < rows; ++row)
    dh += getBox(0, row)->getYFill();
  height = 0;
  h1 = 0;
  for (row = 0; row < rows; ++row) {
    t1 = 0;
    for (col = 0; col < cols; ++col) {
      if (getBox(col, row)->getYFill() > 0)
	t2 = (get(col, row)->getHeight() * dh) / getBox(col, row)->getYFill();
      else
	t2 = get(col, row)->getHeight();
      if (t2 > t1)
	t1 = t2;
    }
    if (getBox(0, row)->getYFill() > 0) {
      if (t1 > h1)
	h1 = t1;
    } else {
      height += t1;
    }
  }
  height += h1 + top + bottom + 2 * borderWidth;
}

void LTKBox::layout2(int x1, int y1, int width1, int height1) {
  int *widths, *heights;
  int col, row;
  int dw1, dw2, dh1, dh2;
  int tx, ty;

  x = x1 + left;
  y = y1 + top;
  width = width1 - left - right;
  height = height1 - top - bottom;

  if (cols == 1 && rows == 1 && !get(0, 0)->isBox()) {
    get(0, 0)->layout2(x + borderWidth, y + borderWidth,
		       width - 2 * borderWidth, height - 2 * borderWidth);
    return;
  }

  dw1 = width - 2 * borderWidth;
  dw2 = 0;
  widths = new int[cols];
  for (col = 0; col < cols; ++col) {
    widths[col] = get(col, 0)->getWidth();
    for (row = 1; row < rows; ++row) {
      if (get(col, row)->getWidth() > widths[col])
	widths[col] = get(col, row)->getWidth();
    }
    if (getBox(col, 0)->getXFill() > 0)
      dw2 += getBox(col, 0)->getXFill();
    else
      dw1 -= widths[col];
  }
  if (dw2 > 0) {
    for (col = 0; col < cols; ++col) {
      if (getBox(col, 0)->getXFill() > 0)
	widths[col] = (getBox(col, 0)->getXFill() * dw1) / dw2;
    }
  }

  dh1 = height - 2 * borderWidth;
  dh2 = 0;
  heights = new int[rows];
  for (row = 0; row < rows; ++row) {
    heights[row] = get(0, row)->getHeight();
    for (col = 1; col < cols; ++col) {
      if (get(col, row)->getHeight() > heights[row])
	heights[row] = get(col, row)->getHeight();
    }
    if (getBox(0, row)->getYFill() > 0)
      dh2 += getBox(0, row)->getYFill();
    else
      dh1 -= heights[row];
  }
  if (dh2 > 0) {
    for (row = 0; row < rows; ++row) {
      if (getBox(0, row)->getYFill() > 0)
	heights[row] = (getBox(0, row)->getYFill() * dh1) / dh2;
    }
  }

  tx = x + borderWidth;
  for (col = 0; col < cols; ++col) {
    ty = y + borderWidth;
    for (row = 0; row < rows; ++row) {
      get(col, row)->layout2(tx, ty, widths[col], heights[row]);
      ty += heights[row];
    }
    tx += widths[col];
  }

  delete[] widths;
  delete[] heights;
}

void LTKBox::layout3() {
  int col, row;

  for (col = 0; col < cols; ++col)
    for (row = 0; row < rows; ++row)
      get(col, row)->layout3();
}

void LTKBox::map() {
  int col, row;

  for (col = 0; col < cols; ++col)
    for (row = 0; row < rows; ++row)
      get(col, row)->map();
}

void LTKBox::redraw() {
  int col, row;

  ltkDrawBorder(getDisplay(), getParent()->getXWindow(),
		getBrightGC(), getDarkGC(), getBgGC(),
		x, y, width, height, border);
  for (col = 0; col < cols; ++col) {
    for (row = 0; row < rows; ++row) {
      if (get(col, row)->isBox())
	get(col, row)->redraw();
    }
  }
}
