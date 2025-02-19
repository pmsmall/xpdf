//========================================================================
//
// LTKWindow.cc
//
//========================================================================

#pragma implementation

#include <stdlib.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <LTKConfig.h>
#include <LTKApp.h>
#include <LTKWindow.h>
#include <LTKWidget.h>
#include <LTKBox.h>
#include <LTKBorder.h>

LTKWindow::LTKWindow(LTKApp *app1, char *title1, LTKWindowKeyCbk keyCbk1,
		     LTKBox *box1) {
  app = app1;
  title = new String(title1);
  width = 0;
  height = 0;
  widgets = NULL;
  box = box1;
  box->setParent(this);
  keyCbk = keyCbk1;
  keyWidget = NULL;
  xwin = None;
  next = NULL;
  app->addWindow(this);
}

LTKWindow::LTKWindow(LTKWindow *window) {
  app = window->app;
  title = window->title->copy();
  width = 0;
  height = 0;
  widgets = NULL;
  box = (LTKBox *)window->box->copy();
  box->setParent(this);
  keyCbk = window->keyCbk;
  keyWidget = NULL;
  xwin = None;
  fgGC = bgGC = brightGC = darkGC = None;
  fontStruct = None;
  next = NULL;
  app->addWindow(this);
}

LTKWindow::~LTKWindow() {
  XEvent event;
  LTKWidget *w1;

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
  app->delWindow(this);
  delete title;
  while (widgets) {
    w1 = widgets;
    widgets = widgets->getNext();
    delete w1;
  }
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
    if (widget->getName() && !widget->getName()->cmp(name))
      break;
  }
  return widget;
}

Boolean LTKWindow::checkFills(char **err) {
  // outer box must either have both xfill > 0 and yfill > 0
  // (i.e., the window is resizable), or xfill = yfill = 0
  // (i.e., the window is not resizable)
  if ((box->getXFill() > 0) != (box->getYFill() > 0)) {
    *err = "outer box must have matched xfill and yfill";
    return false;
  }

  // check consistency of box hierarchy
  if (!box->checkFills(err))
    return false;

  // everything checked out ok
  *err = "";
  return true;
}

void LTKWindow::layout(int width1, int height1) {
  XColor bgXcol;
  XGCValues gcValues;
  int minWidth, minHeight;
  char *title1;
  XSizeHints *sizeHints;
  XWMHints *wmHints;
  XClassHint *classHints;
  XTextProperty windowName, iconName;

  // create window and GC's so widgets can use font info, etc.
  if (xwin == None) {
    display = app->getDisplay();
    screenNum = app->getScreenNum();
    fgColor = app->getColorResource("foreground", LTK_FOREGROUND,
				    BlackPixel(display, screenNum), NULL);
    bgColor = app->getColorResource("background", LTK_BACKGROUND,
				    WhitePixel(display, screenNum), &bgXcol);
    xwin = XCreateSimpleWindow(display, RootWindow(display, screenNum),
			       0, 0, 1, 1, 0,
			       fgColor, bgColor);
    XSelectInput(display, xwin, ExposureMask | StructureNotifyMask |
		 ButtonPressMask | KeyPressMask);
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
  }

  // do layout
  box->layout1();
  minWidth = box->getWidth();
  minHeight = box->getHeight();
  if (box->getXFill() > 0 && box->getYFill() > 0) {
    if (width1 < box->getWidth())
      width1 = box->getWidth();
    if (height1 < box->getHeight())
      height1 = box->getHeight();
    box->layout2(0, 0, width1, height1);
  } else {
    box->layout2(0, 0, box->getWidth(), box->getHeight());
  }
  width = box->getWidth();
  height = box->getHeight();

  // set the real window size
  XResizeWindow(display, xwin, width, height);

  // finish the layout and create the widget subwindows
  box->layout3();

  // set window properties
  sizeHints = XAllocSizeHints();
  wmHints = XAllocWMHints();
  classHints = XAllocClassHint();
  if (sizeHints && wmHints && classHints) {
    title1 = title->getCString();
    XStringListToTextProperty(&title1, 1, &windowName);
    XStringListToTextProperty(&title1, 1, &iconName);
    sizeHints->flags = PPosition | PSize | PMinSize;
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

void LTKWindow::map() {
  XEvent event;

  box->map();
  XMapWindow(display, xwin);
  do {
    XWindowEvent(display, xwin, ExposureMask, &event);
  } while (!(event.type == Expose &&
	     event.xexpose.window == xwin &&
	     event.xexpose.count == 0));
  XPutBackEvent(display, &event);
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

void LTKWindow::setCursor(uint cursor) {
  XDefineCursor(display, xwin, XCreateFontCursor(display, cursor));
}
