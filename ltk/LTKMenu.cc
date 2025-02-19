//========================================================================
//
// LTKMenu.cc
//
// Menus and menu items.
//
// Copyright 1997 Derek B. Noonburg.
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
#include <X11/cursorfont.h>
#include "gmem.h"
#include "LTKConfig.h"
#include "LTKApp.h"
#include "LTKWindow.h"
#include "LTKBorder.h"
#include "LTKMenu.h"

#ifdef XlibSpecificationRelease
#if XlibSpecificationRelease < 5
typedef char *XPointer;
#endif
#else
typedef char *XPointer;
#endif

//------------------------------------------------------------------------

#define horizBorder  2
#define horizSpace  12
#define arrowSize   10

//------------------------------------------------------------------------
// menu design
// -----------
//
// X coordinates:
//
//    /-- main border
//    | /-- selection border
//    | |               /-- shortcut
//    | |               |    /-- submenu arrow
//    | |               |    |   /-- selection border
//    | |               |    |   | /-- main border
//    | |               |    |   | |
//    v v               v    v   v v  
//   | |                          | |
//   | +--------------------------+ |
//   | | +----------------------+ | |
//   | | | ...text...        |> | | |
//   | | +----------------------+ | |
//   | +--------------------------+ |
//   | | +----------------------+ | |
//   | | | ...text...  shortcut | | |
//   | | +----------------------+ | |
//   | +--------------------------+ |
//   | |                          | |
//   A B C D        E  F      G H I J
//   
//   A = 0
//   B = A + ltkBorderWidth
//   C = B + ltkBorderWidth
//   D = C + horizBorder
//   E = D + max{text widths}
//   F = E + horizSpace
//   G = F + max{arrowSize, shortcut widths}
//   H = G + horizBorder
//   I = H + ltkBorderWidth
//   J = I + ltkBorderWidth
//
//   total width = J
//               = 4*ltkBorderWidth + 2*horizBorder + horizSpace
//                 + max{text widths}
//                 + max{arrowSize, shortcut widths}
//
//------------------------------------------------------------------------

static Bool isExposeEvent(Display *display, XEvent *e, XPointer w);

//------------------------------------------------------------------------
// LTKMenu
//------------------------------------------------------------------------

LTKMenu::LTKMenu(char *title1, int numItems1, ...) {
  va_list args;
  int i;

  title = title1;
  numItems = numItems1;
  items = (LTKMenuItem **)gmalloc(numItems * sizeof(LTKMenuItem *));
  va_start(args, numItems1);
  for (i = 0; i < numItems; ++i) {
    items[i] = va_arg(args, LTKMenuItem *);
    items[i]->setParent(this);
  }
  xwin = None;
  widget = NULL;
}

LTKMenu::~LTKMenu() {
  int i;

  if (xwin != None)
    unpost();
  for (i = 0; i < numItems; ++i)
    delete items[i];
  gfree(items);
}

void LTKMenu::post(LTKWindow *win1, int x1, int y1, LTKMenu *parent1) {
  XFontStruct *fontStruct;
  XCharStruct extents;
  XSetWindowAttributes attr;
  XEvent event;
  int w, h, shortcutW, direction, ascent, descent;
  int i;
  GBool haveSubmenus;

  // parent menu
  parent = parent1;

  // parent window
  win = win1;
  display = win->getDisplay();
  fgGC = win->getFgGC();
  bgGC = win->getBgGC();
  brightGC = win->getBrightGC();
  darkGC = win->getDarkGC();
  fontStruct = win->getXFontStruct();

  // compute width and height
  width = 0;
  height = 0;
  itemHeight = fontStruct->ascent + fontStruct->descent;
  if (itemHeight < arrowSize)
    itemHeight = arrowSize;
  if (title) {
    XTextExtents(fontStruct, title, strlen(title),
		 &direction, &ascent, &descent, &extents);
    if (extents.width > width)
      width = extents.width;
    height += itemHeight + ltkBorderWidth + 2 + 2 * ltkBorderWidth;
  }
  shortcutW = 0;
  haveSubmenus = gFalse;
  for (i = 0; i < numItems; ++i) {
    if (items[i]->text) {
      XTextExtents(fontStruct, items[i]->text, strlen(items[i]->text),
		   &direction, &ascent, &descent, &extents);
      if (extents.width > width)
	width = extents.width;
      height += itemHeight + ltkBorderWidth;
    } else {
      height += 2 + ltkBorderWidth;
    }
    if (items[i]->submenu) {
      haveSubmenus = gTrue;
    } else if (items[i]->shortcut) {
      XTextExtents(fontStruct, items[i]->shortcut, strlen(items[i]->shortcut),
		   &direction, &ascent, &descent, &extents);
      if (extents.width > shortcutW)
	shortcutW = extents.width;
    }
  }
  width += 2*ltkBorderWidth + horizBorder + horizSpace;
  if (!haveSubmenus || shortcutW > arrowSize)
    width += shortcutW;
  else 
    width += arrowSize;
  width += 2*ltkBorderWidth + horizBorder;
  height += 3*ltkBorderWidth;

  // compute position
  w = win->getApp()->getDisplayWidth();
  h = win->getApp()->getDisplayHeight();
  x = x1;
  if (x + width > w)
    x = w - width;
  if (x < 0)
    x = 0;
  y = y1;
  if (y + height > h)
    y = h - height;
  if (y < 0)
    y = 0;

  // create X window
  attr.background_pixel = win->getBgColor();
  attr.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask;
  attr.override_redirect = True;
  xwin = XCreateWindow(display,
		       RootWindow(display, win->getScreenNum()),
		       x, y, width, height, 0, CopyFromParent,
		       InputOutput, CopyFromParent,
		       CWBackPixel | CWEventMask | CWOverrideRedirect,
		       &attr);

  // map it
  XMapWindow(display, xwin);
  XPeekIfEvent(display, &event, &isExposeEvent, (XPointer)this);

  // grab the pointer
  XGrabPointer(display, xwin, False,
	       ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
	         PointerMotionHintMask,
	       GrabModeAsync, GrabModeAsync, None,
	       XCreateFontCursor(display, XC_sb_left_arrow), CurrentTime);

  // register with app
  win->getApp()->setMenu(this);

  // no item selected yet
  currentItem = -1;
  currentSubmenu = NULL;
}

static Bool isExposeEvent(Display *display, XEvent *e, XPointer w) {
  return e->type == Expose &&
         e->xexpose.window == ((LTKMenu *)w)->getXWindow();
}

void LTKMenu::repost() {
  // grab the pointer
  XGrabPointer(display, xwin, False,
	       ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
	         PointerMotionHintMask,
	       GrabModeAsync, GrabModeAsync, None,
	       XCreateFontCursor(display, XC_sb_left_arrow), CurrentTime);

  // register with app
  win->getApp()->setMenu(this);
}

void LTKMenu::unpost() {
  XEvent event;

  // unpost any submenus
  if (currentSubmenu) {
    currentSubmenu->unpost();
    currentSubmenu = NULL;
  }

  // let go of the pointer
  XUngrabPointer(display, CurrentTime);

  // kill the window
  XUnmapWindow(display, xwin);
  XDestroyWindow(display, xwin);

  // flush any leftover events
  XSync(display, False);
  while (XCheckWindowEvent(display, xwin, 0xffffffff, &event)) ;

  // mark the menu as unposted
  xwin = None;
  win->getApp()->setMenu(NULL);
}

void LTKMenu::done() {
  if (parent)
    parent->done();
  else
    unpost();
}

void LTKMenu::redraw() {
  XCharStruct extents;
  int direction, ascent, descent;
  int textBase, y1, x2, y2;
  int i;

  // draw menu border
  ltkDrawBorder(display, xwin, brightGC, darkGC, bgGC,
		0, 0, width, height, ltkBorderRaised);

  // draw title
  textBase = win->getXFontStruct()->ascent;
  y1 = 2 * ltkBorderWidth;
  if (title) {
    XTextExtents(win->getXFontStruct(), title, strlen(title),
		 &direction, &ascent, &descent, &extents);
    x2 = (width - extents.width) / 2;
    XDrawString(display, xwin, fgGC,
		x2, y1 + textBase, title, strlen(title));
    y1 += itemHeight + ltkBorderWidth;
    ltkDrawSplitBorder(display, xwin, brightGC, darkGC, bgGC,
		       0, y1, width, 0, ltkBorderSunken);
    y1 += 2 + 2 * ltkBorderWidth;
  }

  // draw items
  for (i = 0; i < numItems; ++i) {
    if (items[i]->text) {
      XDrawString(display, xwin, fgGC,
		  2*ltkBorderWidth + horizBorder, y1 + textBase,
		  items[i]->text, strlen(items[i]->text));
      if (items[i]->submenu) {
	x2 = width - 2*ltkBorderWidth - horizBorder - arrowSize;
	y2 = y1 + (itemHeight - arrowSize) / 2;
	ltkDrawTriBorder(display, xwin, brightGC, darkGC, bgGC,
			 x2, y2, arrowSize, arrowSize,
			 ltkTriRight, ltkBorderRaised);
      } else if (items[i]->shortcut) {
	XTextExtents(win->getXFontStruct(),
		     items[i]->shortcut, strlen(items[i]->shortcut),
		     &direction, &ascent, &descent, &extents);
	x2 = width - 2*ltkBorderWidth - horizBorder - extents.width;
	XDrawString(display, xwin, fgGC, x2, y1 + textBase,
		    items[i]->shortcut, strlen(items[i]->shortcut));
      }
      y1 += itemHeight + ltkBorderWidth;
    } else {
      ltkDrawDivider(display, xwin, brightGC, darkGC, bgGC,
		     2*ltkBorderWidth + horizBorder - 1, y1,
		     width - 4*ltkBorderWidth - 2*horizBorder + 2, 0,
		     ltkBorderRaised);
      y1 += 2 + ltkBorderWidth;
    }
  }
}

void LTKMenu::buttonPress(int mx, int my, int button, GBool dblClick) {
  done();
  if (currentItem >= 0 && items[currentItem]->cbk) {
    (*items[currentItem]->cbk)(items[currentItem]);
  }

  // The matching button release event may not happen because
  // the window is destroyed by done(), so tell the LTKApp that
  // the button is no longer pressed.
  win->getApp()->clearButton();
}

void LTKMenu::buttonRelease(int mx, int my, int button, GBool click) {
  if (!click) {
    done();
    if (currentItem >= 0 && items[currentItem]->cbk) {
      (*items[currentItem]->cbk)(items[currentItem]);
    }
  }
}

void LTKMenu::mouseMove(int mx, int my, int btn) {
  int y1, i, j;

  y1 = ltkBorderWidth;
  if (title)
    y1 += itemHeight + ltkBorderWidth + 2 + 2 * ltkBorderWidth;
  j = -1;
  if (mx >= 0 && mx < width && my >= y1) {
    for (i = 0; i < numItems; ++i) {
      if (items[i]->text) {
	if (my < y1 + itemHeight + ltkBorderWidth) {
	  j = i;
	  break;
	}
	y1 += itemHeight + ltkBorderWidth;
      } else {
	if (my < y1 + 2 + ltkBorderWidth)
	  break;
	y1 += 2 + ltkBorderWidth;
      }
    }
  }
  if (j != currentItem && (j >= 0 || !currentSubmenu)) {
    if (currentItem >= 0) {
      if (currentSubmenu) {
	currentSubmenu->unpost();
	currentSubmenu = NULL;
	repost();
      }
      ltkDrawBorder(display, xwin, brightGC, darkGC, bgGC,
		    ltkBorderWidth, currentY,
		    width - 2 * ltkBorderWidth,
		    itemHeight + 2*ltkBorderWidth,
		    ltkBorderNone);
    }
    currentItem = j;
    currentY = y1;
    if (currentItem >= 0) {
      ltkDrawBorder(display, xwin, brightGC, darkGC, bgGC,
		    ltkBorderWidth, currentY,
		    width - 2 * ltkBorderWidth,
		    itemHeight + 2*ltkBorderWidth,
		    ltkBorderRaised);
      if (items[currentItem]->submenu) {
	currentSubmenu = items[currentItem]->submenu;
	currentSubmenu->post(win, x + width - 2*ltkBorderWidth,
			     y + currentY, this);
      }
    }
  }
  if (j < 0 && parent)
    parent->mouseMove(x + mx - parent->x, y + my - parent->y, btn);
}

//------------------------------------------------------------------------
// LTKMenuItem
//------------------------------------------------------------------------

LTKMenuItem::LTKMenuItem(char *text1, char *shortcut1, int itemNum1,
			 LTKMenuCbk cbk1, LTKMenu *submenu1) {
  text = text1;
  shortcut = shortcut1;
  itemNum = itemNum1;
  cbk = cbk1;
  submenu = submenu1;
  parent = NULL;
}
