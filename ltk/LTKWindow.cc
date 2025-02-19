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
#include <LTKConfig.h>
#include <LTKApp.h>
#include <LTKWindow.h>
#include <LTKWidget.h>
#include <LTKBox.h>
#include <LTKBorder.h>

#ifdef XlibSpecificationRelease
#if XlibSpecificationRelease < 5
typedef char *XPointer;
#endif
#else
typedef char *XPointer;
#endif

static Bool isExposeEvent(Display *display, XEvent *e, XPointer win);

LTKWindow::LTKWindow(LTKApp *app1, GBool dialog1, char *title1,
		     LTKBox *box1) {
  app = app1;
  dialog = dialog1;
  title = new GString(title1);
  width = 0;
  height = 0;
  widgets = NULL;
  box = box1;
  box->setParent(this);
  keyCbk = NULL;
  propCbk = NULL;
  layoutCbk = NULL;
  keyWidget = NULL;
  overWin = NULL;
  xwin = None;
  eventMask = ExposureMask | StructureNotifyMask | VisibilityChangeMask |
              ButtonPressMask;
  fgGC = bgGC = brightGC = darkGC = None;
  fontStruct = NULL;
  next = NULL;
  app->addWindow(this);
}

LTKWindow::LTKWindow(LTKWindow *window) {
  app = window->app;
  dialog = window->dialog;
  title = window->title->copy();
  width = 0;
  height = 0;
  widgets = NULL;
  box = (LTKBox *)window->box->copy();
  box->setParent(this);
  keyCbk = window->keyCbk;
  propCbk = window->propCbk;
  layoutCbk = window->layoutCbk;
  keyWidget = NULL;
  overWin = NULL;
  xwin = None;
  fgGC = bgGC = brightGC = darkGC = None;
  fontStruct = NULL;
  next = NULL;
  app->addWindow(this);
}

LTKWindow::~LTKWindow() {
  XEvent event;

  if (xwin) {
    XFreeFont(display, fontStruct);
    XFreeGC(display, fgGC);
    XFreeGC(display, bgGC);
    XFreeGC(display, brightGC);
    XFreeGC(display, darkGC);
    XUnmapWindow(display, xwin);
    XDestroyWindow(display, xwin);
    XSync(display, True);
    while (XCheckWindowEvent(display, xwin, 0xffffffff, &event)) ;
  }
  app->setGrabWin(NULL);
  app->delWindow(this);
  delete title;
  delete box;
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

LTKWidget *LTKWindow::findWidget(Window xwin1) {
  LTKWidget *widget;

  for (widget = widgets; widget; widget = widget->getNext()) {
    if (widget->getXWindow() == xwin1)
      break;
  }
  return widget;
}

LTKWidget *LTKWindow::findWidget(char *name) {
  LTKWidget *widget;

  for (widget = widgets; widget; widget = widget->getNext()) {
    if (widget->getName() && !strcmp(widget->getName(), name))
      break;
  }
  return widget;
}

void LTKWindow::setKeyCbk(LTKWindowKeyCbk cbk) {
  keyCbk = cbk;
  if (keyCbk)
    eventMask |= KeyPressMask;
  else
    eventMask &= ~KeyPressMask;
  if (xwin != None)
    XSelectInput(display, xwin, eventMask);
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
  XGCValues gcValues;
  int minWidth, minHeight;
  int width2, height2;
  char *title1;
  XSizeHints *sizeHints;
  XWMHints *wmHints;
  XClassHint *classHints;
  XTextProperty windowName, iconName;
  GBool newWin;

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
    XSelectInput(display, xwin, eventMask);
    gcValues.foreground = fgColor;
    gcValues.background = bgColor;
    gcValues.line_width = 0;
    gcValues.line_style = LineSolid;
    gcValues.graphics_exposures = False;
    fgGC = XCreateGC(display, xwin,
		     GCForeground | GCBackground | GCLineWidth | GCLineStyle |
		     GCGraphicsExposures,
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
    fontStruct = app->getFontResource("font", LTK_WIN_FONT);
    XSetFont(display, fgGC, fontStruct->fid);
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

  // set window properties
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
      classHints->res_name = app->getAppName()->getCString();
      classHints->res_class = app->getAppName()->getCString();
      XSetWMProperties(display, xwin, &windowName, &iconName,
		       NULL, 0, sizeHints, wmHints, classHints);
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
      XFree((void *)children);
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
  Window xwin;

  win = (LTKWindow *)w;
  xwin = e->xexpose.window;
  return e->type == Expose &&
	 (win->getXWindow() == xwin || win->findWidget(xwin));
}

void LTKWindow::redraw() {
  box->redraw();
}

void LTKWindow::keyPress(KeySym key, char *s, int n) {
  if (keyWidget)
    keyWidget->keyPress(key, s, n);
  else if (keyCbk)
    (*keyCbk)(this, key, s, n);
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
  gcValues.graphics_exposures = False;
  gc = XCreateGC(display, xwin,
		 GCForeground | GCLineWidth | GCLineStyle |
		 GCGraphicsExposures,
		 &gcValues);
  return gc;
}

void LTKWindow::setCursor(Guint cursor) {
  XDefineCursor(display, xwin, XCreateFontCursor(display, cursor));
}
