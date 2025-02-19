//========================================================================
//
// LTKWindow.cc
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
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#ifdef HAVE_X11_XPM_H
#include <X11/xpm.h>
#endif
#include "LTKConfig.h"
#include "LTKApp.h"
#include "LTKMenu.h"
#include "LTKWindow.h"
#include "LTKWidget.h"
#include "LTKBox.h"
#include "LTKBorder.h"

#ifdef XlibSpecificationRelease
#if XlibSpecificationRelease < 5
typedef char *XPointer;
#endif
#else
typedef char *XPointer;
#endif

// This is used only to set the no-decorations hint.
typedef struct {
  Gulong flags;
  Gulong functions;
  Gulong decorations;
  long input_mode;
  Gulong status;
} MotifWmHints;
#define MWM_HINTS_DECORATIONS (1L << 1)

static Bool isExposeEvent(Display *display, XEvent *e, XPointer w);

LTKWindow::LTKWindow(LTKApp *app1, GBool dialog1, char *title1,
		     char **iconData1, char *defaultWidgetName,
		     LTKBox *box1) {
  app = app1;
  dialog = dialog1;
  title = new GString(title1);
  iconData = iconData1;
  width = 0;
  height = 0;
  widgets = NULL;
  box = box1;
  box->setParent(this);
  selection = NULL;
  savedCursor = None;
  busyCursorNest = 0;
  menu = NULL;
  keyCbk = NULL;
  propCbk = NULL;
  layoutCbk = NULL;
  decorated = gTrue;
  if (defaultWidgetName)
    defaultWidget = findWidget(defaultWidgetName);
  else
    defaultWidget = NULL;
  keyWidget = NULL;
  selectionWidget = NULL;
  pasteWidget = NULL;
  overWin = NULL;
  xwin = None;
  eventMask = ExposureMask | StructureNotifyMask | VisibilityChangeMask |
              ButtonPressMask | ButtonReleaseMask | KeyPressMask;
  installCmap = gFalse;
  fgGC = bgGC = brightGC = darkGC = xorGC = None;
  fontStruct = NULL;
  next = NULL;
  app->addWindow(this);
}

LTKWindow::~LTKWindow() {
  if (xwin) {
    XFreeFont(display, fontStruct);
    XFreeGC(display, fgGC);
    XFreeGC(display, bgGC);
    XFreeGC(display, brightGC);
    XFreeGC(display, darkGC);
    XFreeGC(display, xorGC);
    if (installCmap)
      XFreeColormap(display, colormap);
    XUnmapWindow(display, xwin);
    XDestroyWindow(display, xwin);
  }
  app->setGrabWin(NULL);
  app->delWindow(this);
  delete title;
  delete box;
  if (selection)
    delete selection;
  if (menu)
    delete menu;
}

LTKWidget *LTKWindow::addWidget(LTKWidget *widget) {
  widget->setNext(widgets);
  widgets = widget;
  return widget;
}

LTKWidget *LTKWindow::delWidget(LTKWidget *widget) {
  LTKWidget *w1, *w2;

  for (w1 = NULL, w2 = widgets;
       w2 && w2 != widget;
       w1 = w2, w2 = w2->getNext()) ;
  if (w2 == widget) {
    if (w1)
      w1->setNext(w2->getNext());
    else
      widgets = w2->getNext();
    w2->setNext(NULL);
    return w2;
  }
  return NULL;
}

LTKWidget *LTKWindow::findWidget(char *name) {
  LTKWidget *widget;

  for (widget = widgets; widget; widget = widget->getNext()) {
    if (widget->getName() && !strcmp(widget->getName(), name))
      break;
  }
  return widget;
}

void LTKWindow::setPropChangeCbk(LTKWindowPropCbk cbk) {
  propCbk = cbk;
  if (propCbk)
    eventMask |= PropertyChangeMask;
  else
    eventMask &= ~PropertyChangeMask;
  if (xwin != None)
    XSelectInput(display, xwin, eventMask);
}

GBool LTKWindow::checkFills(char **err) {
  // outer box must either have both xfill > 0 and yfill > 0
  // (i.e., the window is resizable), or xfill = yfill = 0
  // (i.e., the window is not resizable)
  if ((box->getXFill() > 0) != (box->getYFill() > 0)) {
    *err = "outer box must have matched xfill and yfill";
    return gFalse;
  }

  // check consistency of box hierarchy
  if (!box->checkFills(err))
    return gFalse;

  // everything checked out ok
  *err = "";
  return gTrue;
}

void LTKWindow::layout(int x, int y, int width1, int height1) {
  XColor bgXcol;
  XColor xcol;
  XGCValues gcValues;
  int minWidth, minHeight;
  int width2, height2;
  char *title1;
  XSizeHints *sizeHints;
  XWMHints *wmHints;
  XClassHint *classHints;
  XTextProperty windowName, iconName;
  Atom protocol;
  GBool newWin;
  Atom motifHintsAtom;
  MotifWmHints motifHints;

  // create window and GC's so widgets can use font info, etc.
  if (xwin == None) {
    newWin = gTrue;
    display = app->getDisplay();
    screenNum = app->getScreenNum();
    fgColor = app->getColorResource("foreground", LTK_FOREGROUND,
				    BlackPixel(display, screenNum), NULL);
    bgColor = app->getColorResource("background", LTK_BACKGROUND,
				    WhitePixel(display, screenNum), &bgXcol);
    xwin = XCreateSimpleWindow(display, RootWindow(display, screenNum),
			       (x < 0) ? 0 : x, (y < 0) ? 0 : y, 1, 1, 0,
			       fgColor, bgColor);
    app->registerXWindow(xwin, this, NULL);
    XSelectInput(display, xwin, eventMask);
    gcValues.foreground = fgColor;
    gcValues.background = bgColor;
    gcValues.line_width = 0;
    gcValues.line_style = LineSolid;
    gcValues.graphics_exposures = True;
    fontStruct = app->getFontResource("font", LTK_WIN_FONT);
    gcValues.font = fontStruct->fid;
    fgGC = XCreateGC(display, xwin,
		     GCForeground | GCBackground | GCLineWidth | GCLineStyle |
		     GCGraphicsExposures | GCFont,
		     &gcValues);
    gcValues.foreground = bgColor;
    bgGC = XCreateGC(display, xwin,
		     GCForeground | GCBackground | GCLineWidth | GCLineStyle |
		     GCGraphicsExposures,
		     &gcValues);
    gcValues.foreground = ltkGetBrightColor(display, screenNum,
					    &bgXcol, fgColor);
    brightGC = XCreateGC(display, xwin,
			 GCForeground | GCBackground | GCLineWidth |
			 GCLineStyle | GCGraphicsExposures,
			 &gcValues);
    gcValues.foreground = ltkGetDarkColor(display, screenNum,
					  &bgXcol, fgColor);
    darkGC = XCreateGC(display, xwin,
		       GCForeground | GCBackground | GCLineWidth |
		       GCLineStyle | GCGraphicsExposures,
		       &gcValues);
    gcValues.foreground = fgColor ^ bgColor;
    gcValues.function = GXxor;
    xorGC = XCreateGC(display, xwin,
		      GCForeground | GCBackground | GCLineWidth | GCLineStyle |
		      GCGraphicsExposures | GCFunction,
		      &gcValues);
    colormap = DefaultColormap(display, screenNum);
    if (installCmap) {
      // ensure that BlackPixel and WhitePixel are reserved in the
      // new colormap
      xcol.red = xcol.green = xcol.blue = 0;
      XAllocColor(display, colormap, &xcol);
      xcol.red = xcol.green = xcol.blue = 65535;
      XAllocColor(display, colormap, &xcol);
      colormap = XCopyColormapAndFree(display, colormap);
      XSetWindowColormap(display, xwin, colormap);
    }
  } else {
    newWin = gFalse;
  }

  // do layout
  box->layout1();
  minWidth = box->getWidth();
  minHeight = box->getHeight();
  if (box->getXFill() > 0 && box->getYFill() > 0) {
    width2 = (width1 < minWidth) ? minWidth : width1;
    height2 = (height1 < minHeight) ? minHeight : height1;
    box->layout2(0, 0, width2, height2);
  } else {
    box->layout2(0, 0, minWidth, minHeight);
  }
  width = box->getWidth();
  height = box->getHeight();

  // set the real window size
  if (newWin)
    XResizeWindow(display, xwin, width, height);

  // finish the layout and create the widget subwindows
  box->layout3();

  // set window properties and protocols
  if (newWin) {
    sizeHints = XAllocSizeHints();
    wmHints = XAllocWMHints();
    classHints = XAllocClassHint();
    if (sizeHints && wmHints && classHints) {
      title1 = title->getCString();
      XStringListToTextProperty(&title1, 1, &windowName);
      XStringListToTextProperty(&title1, 1, &iconName);
      sizeHints->flags = PMinSize;
      sizeHints->flags |= (x >= 0 || y >= 0) ? USPosition : PPosition;
      sizeHints->flags |= (width1 >= 0 || height1 >= 0) ? USSize : PSize;
      sizeHints->min_width = minWidth;
      sizeHints->min_height = minHeight;
      if (!(box->getXFill() > 0 && box->getYFill() > 0)) {
	sizeHints->flags |= PMaxSize;
	sizeHints->max_width = minWidth;
	sizeHints->max_height = minHeight;
      }
      wmHints->input = True;
      wmHints->initial_state = NormalState;
      wmHints->flags = InputHint | StateHint;
#ifdef HAVE_X11_XPM_H
      if (iconData) {
	if (XpmCreatePixmapFromData(display, xwin, iconData,
				    &wmHints->icon_pixmap, NULL, NULL) ==
	    XpmSuccess)
	  wmHints->flags |= IconPixmapHint;
      }
#endif
      classHints->res_name = app->getAppName()->getCString();
      classHints->res_class = app->getAppName()->getCString();
      XSetWMProperties(display, xwin, &windowName, &iconName,
		       NULL, 0, sizeHints, wmHints, classHints);
    }
    protocol = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, xwin, &protocol, 1);
    if (!decorated) {
      motifHintsAtom = XInternAtom(display, "_MOTIF_WM_HINTS", False);
      motifHints.flags = MWM_HINTS_DECORATIONS;
      motifHints.decorations = False;
      XChangeProperty(display, xwin, motifHintsAtom, motifHintsAtom,
		      32, PropModeReplace, (Guchar *)&motifHints,
		      sizeof(MotifWmHints) / sizeof(long));
    }
  }

  // call layout callback
  if (layoutCbk)
    (*layoutCbk)(this);
}

void LTKWindow::layoutDialog(LTKWindow *overWin1, int width1, int height1) {
  Window w1, w2, root;
  Window *children;
  Guint numChildren;
  int x, y;
  Guint w, h, bw, depth;

  // save the over-window
  overWin = overWin1;

  // layout the dialog
  layout(0, 0, width1, height1);

  // find the window manager's outermost parent of <overWin>
  w2 = overWin->getXWindow();
  do {
    w1 = w2;
    if (!XQueryTree(display, w2, &root, &w2, &children, &numChildren))
      break;
    if (numChildren > 0)
      XFree((XPointer)children);
  } while (w2 != root);

  // center the dialog over <overWin>
  XGetGeometry(display, w1, &root, &x, &y, &w, &h, &bw, &depth);
  if ((Guint)width < w)
    x += (w - width) / 2;
  if ((Guint)height < h)
    y += (h - height) / 2;
  XMoveWindow(display, xwin, x, y);
}

void LTKWindow::map() {
  XEvent event;

  box->map();
  XMapWindow(display, xwin);
  XPeekIfEvent(display, &event, &isExposeEvent, (XPointer)this);
  if (dialog)
    app->setGrabWin(this);
}

static Bool isExposeEvent(Display *display, XEvent *e, XPointer w) {
  LTKWindow *win;
  LTKWidget *widget;
  Window xwin;

  win = (LTKWindow *)w;
  xwin = e->xany.window;
  return e->type == Expose &&
         win->getApp()->findWindow(xwin, &widget) == win;
}

void LTKWindow::redraw() {
  box->redraw();
}

void LTKWindow::redrawBackground() {
  box->redrawBackground();
}

void LTKWindow::setSelection(LTKWidget *widget, GString *text) {
  selectionWidget = widget;
  if (selection)
    delete selection;
  selection = text;
  //~ this shouldn't use CurrentTime (?)
  XSetSelectionOwner(display, XA_PRIMARY, xwin, CurrentTime);
}

void LTKWindow::requestPaste(LTKWidget *widget) {
  Atom prop;

  prop = XInternAtom(display, "XPDF_SELECTION", False);
  //~ this shouldn't use CurrentTime (?)
  XConvertSelection(display, XA_PRIMARY, XA_STRING, prop, xwin, CurrentTime);
  pasteWidget = widget;
}

void LTKWindow::postMenu(int mx, int my) {
  if (menu)
    menu->post(this, mx, my, NULL);
}

void LTKWindow::keyPress(KeySym key, Guint modifiers, char *s, int n) {
  if (keyWidget)
    keyWidget->keyPress(key, modifiers, s, n);
  else if (keyCbk)
    (*keyCbk)(this, key, modifiers, s, n);
  else if (defaultWidget && n > 0 && (s[0] == '\n' || s[0] == '\r'))
    defaultWidget->activateDefault();
}

void LTKWindow::propChange(Atom atom) {
  if (propCbk)
    (*propCbk)(this, atom);
}

GC LTKWindow::makeGC(unsigned long color, int lineWidth, int lineStyle) {
  XGCValues gcValues;
  GC gc;

  gcValues.foreground = color;
  gcValues.line_width = lineWidth;
  gcValues.line_style = lineStyle;
  gcValues.graphics_exposures = True;
  gc = XCreateGC(display, xwin,
		 GCForeground | GCLineWidth | GCLineStyle |
		 GCGraphicsExposures,
		 &gcValues);
  return gc;
}

void LTKWindow::setTitle(GString *title1) {
  XTextProperty windowName, iconName;
  char *title2;

  delete title;
  title = title1;
  title2 = title->getCString();
  if (xwin != None) {
    XStringListToTextProperty(&title2, 1, &windowName);
    XStringListToTextProperty(&title2, 1, &iconName);
    XSetWMName(display, xwin, &windowName);
    XSetWMIconName(display, xwin, &iconName);
  }
}

void LTKWindow::setCursor(Guint cursor) {
  Cursor c;

  if (busyCursorNest == 0) {
    c = XCreateFontCursor(display, cursor);
    XDefineCursor(display, xwin, c);
    XFreeCursor(display, c);
  }
  savedCursor = cursor;
}

void LTKWindow::setDefaultCursor() {
  if (busyCursorNest == 0)
    XUndefineCursor(display, xwin);
  savedCursor = None;
}

void LTKWindow::setBusyCursor(GBool busy) {
  Cursor c;

  if (busy) {
    if (busyCursorNest == 0) {
      c = XCreateFontCursor(display, XC_watch);
      XDefineCursor(display, xwin, c);
      XFreeCursor(display, c);
      XFlush(display);
    }
    ++busyCursorNest;
  } else if (busyCursorNest > 0) {
    --busyCursorNest;
    if (busyCursorNest == 0) {
      if (savedCursor == None) {
	XUndefineCursor(display, xwin);
      } else {
	c = XCreateFontCursor(display, savedCursor);
	XDefineCursor(display, xwin, c);
	XFreeCursor(display, c);
	XFlush(display);
      }
    }
  }
}

void LTKWindow::raise() {
  XMapRaised(display, xwin);
}
