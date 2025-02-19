//========================================================================
//
// LTKMenuButton.cc
//
// Copyright 1999 Derek B. Noonburg
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
#include "gmem.h"
#include "LTKWindow.h"
#include "LTKMenuButton.h"
#include "LTKBorder.h"

#ifdef XlibSpecificationRelease
#if XlibSpecificationRelease < 5
typedef char *XPointer;
#endif
#else
typedef char *XPointer;
#endif

static void menuItemCbk(LTKMenuItem *item);

LTKMenuButton::LTKMenuButton(char *name, int widgetNum, LTKMenu *menu):
    LTKWidget(name, widgetNum) {
  LTKMenuItem *item;
  int i;

  this->menu = menu;
  menuItem = NULL;

  cbks = (LTKMenuCbk *)gmalloc(menu->getNumItems() * sizeof(LTKMenuCbk));
  for (i = 0; i < menu->getNumItems(); ++i) {
    item = menu->getItem(i);
    cbks[i] = item->getCbk();
    item->setCbk(&menuItemCbk);
  }
  menu->setParentWidget(this);
}

LTKMenuButton::~LTKMenuButton() {
  gfree(cbks);
  delete menu;
}

long LTKMenuButton::getEventMask() {
  return LTKWidget::getEventMask() | ButtonPressMask | ButtonReleaseMask;
}

void LTKMenuButton::layout1() {
  XFontStruct *fontStruct;
  XCharStruct extents;
  char *s;
  int direction, ascent, descent;
  int tri;
  int i;

  fontStruct = getXFontStruct();
  textHeight = fontStruct->ascent + fontStruct->descent;
  textBase = fontStruct->ascent;
  textWidth = 0;
  for (i = 0; i < menu->getNumItems(); ++i) {
    if ((s = menu->getItem(i)->getText())) {
      XTextExtents(fontStruct, s, strlen(s),
		   &direction, &ascent, &descent, &extents);
      if (extents.width > textWidth) {
	textWidth = extents.width;
      }
    }
  }
  tri = textHeight < 12 ? textHeight : 12;
  width = textWidth + tri + 16 + 2 * ltkBorderWidth;
  height = textHeight + 4 + 2 * ltkBorderWidth;
}

void LTKMenuButton::redraw() {
  char *s;
  int tri, tx, ty;

  ltkDrawBorder(getDisplay(), xwin, getBrightGC(), getDarkGC(), getBgGC(),
		0, 0, width, height, ltkBorderRaised);
  tri = textHeight < 12 ? textHeight : 12;
  ty = (height - tri) / 2;
  ltkDrawTriBorder(getDisplay(), xwin, getBrightGC(), getDarkGC(), getBgGC(),
		   ltkBorderWidth + 2, ty,
		   tri, tri, ltkTriDown, ltkBorderRaised);
  if (menuItem) {
    s = menuItem->getText();
    tx = tri + 4 + (width - (tri + 4) - textWidth) / 2;
    ty = (height - textHeight) / 2 + textBase;
    XDrawString(getDisplay(), xwin, getFgGC(), tx, ty, s, strlen(s));
  }
}

void LTKMenuButton::buttonPress(int mx, int my, int button, GBool dblClick) {
  Window win, root;
  Window *children;
  Guint numChildren;
  int wx, wy, dx, dy;
  Guint w, h, bw, depth;

  // compute absolute position of button
  wx = x;
  wy = y;
  win = parent->getXWindow();
  do {
    XGetGeometry(getDisplay(), win, &root,
		 &dx, &dy, &w, &h, &bw, &depth);
    if (!XQueryTree(getDisplay(), win, &root, &win, &children, &numChildren)) {
      break;
    }
    if (numChildren > 0) {
      XFree((XPointer)children);
    }
    wx += dx;
    wy += dy;
  } while (win != root);

  menu->post(parent, wx, wy, NULL);
}

void LTKMenuButton::setMenuItem(LTKMenuItem *item) {
  int i;

  menuItem = item;
  XClearWindow(getDisplay(), xwin);
  redraw();
  for (i = 0; i < menu->getNumItems(); ++i) {
    if (item == menu->getItem(i)) {
      if (cbks[i]) {
	(*cbks[i])(item);
      }
      break;
    }
  }
}

static void menuItemCbk(LTKMenuItem *item) {
  ((LTKMenuButton *)item->getParent()->getParentWidget())->setMenuItem(item);
}
