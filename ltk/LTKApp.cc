//========================================================================
//
// LTKApp.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <gtypes.h>
#include <LTKApp.h>
#include <LTKResources.h>
#include <LTKWindow.h>
#include <LTKMisc.h>
#include <LTKWidget.h>

LTKApp::LTKApp(char *appName1, XrmOptionDescRec *opts,
	       int *argc, char *argv[]) {
  int numOpts;
  XrmDatabase cmdLineDB;
  GString *displayName;

  appName = new GString(appName1);
  windows = NULL;
  grabWin = NULL;
  repeatWidget = NULL;
  cmdLineDB = NULL;
  resourceDB = NULL;
  XrmInitialize();
  for (numOpts = 0; opts[numOpts].option; ++numOpts) ;
  ltkGetCmdLineResources(&cmdLineDB, opts, numOpts, appName, argc, argv);
  displayName = ltkGetStringResource(cmdLineDB, appName, "display", "");
  if (!(display = XOpenDisplay(displayName->getCString()))) {
    ltkError("Cannot connect to X server %s\n", XDisplayName(NULL));
    exit(1);
  }
  delete displayName;
  screenNum = DefaultScreen(display);
  ltkGetOtherResources(display, cmdLineDB, &resourceDB);
}

LTKApp::~LTKApp() {
  LTKWindow *w1;

  while (windows) {
    w1 = windows;
    windows = windows->getNext();
    delete w1;
  }
  XCloseDisplay(display);
  delete appName;
}

GString *LTKApp::getStringResource(char *inst, char *def) {
  return ltkGetStringResource(resourceDB, appName, inst, def);
}

unsigned long LTKApp::getColorResource(char *inst,
				       char *def1, unsigned long def2,
				       XColor *xcol) {
  return ltkGetColorResource(resourceDB, appName, inst,
			     display, screenNum, def1, def2, xcol);
}

XFontStruct *LTKApp::getFontResource(char *inst,  char *def) {
  return ltkGetFontResouce(resourceDB, appName, inst, display, screenNum, def);
}

void LTKApp::getGeometryResource(char *inst, int *x, int *y,
				 Guint *width, Guint *height) {
  ltkGetGeometryResource(resourceDB, appName, inst, display, screenNum,
			 x, y, width, height);
}

LTKWindow *LTKApp::addWindow(LTKWindow *w) {
  w->setNext(windows);
  windows = w;
  return w;
}

LTKWindow *LTKApp::delWindow(LTKWindow *w) {
  LTKWindow *w1, *w2;

  for (w1 = NULL, w2 = windows; w2 && w2 != w; w1 = w2, w2 = w2->getNext()) ;
  if (w2 == w) {
    if (w1)
      w1->setNext(w2->getNext());
    else
      windows = w2->getNext();
    w2->setNext(NULL);
    return w2;
  }
  return NULL;
}

LTKWindow *LTKApp::findWindow(Window xwin, LTKWidget **widget) {
  LTKWindow *w;
  Window root;
  Window xparent;
  Window *children;
  unsigned int numChildren;

  root = None;
  xparent = None;
  children = NULL;
  numChildren = 0;
  XQueryTree(display, xwin, &root, &xparent, &children, &numChildren);
  if (numChildren > 0)
    XFree((void *)children);
  *widget = NULL;
  for (w = windows; w; w = w->getNext()) {
    if (w->getXWindow() == xwin)
      break;
    if (w->getXWindow() == xparent) {
      *widget = w->findWidget(xwin);
      break;
    }
  }
  return w;
}

void LTKApp::doEvent(GBool wait) {
  XEvent event;
  int pending;
  LTKWindow *win;
  LTKWidget *widget;
  KeySym key;
  char s[20];
  int n;

 start:
  pending = XPending(display);

  if (pending == 0 && repeatWidget) {
    repeatWidget->repeatEvent();

  } else if (pending > 0 || wait) {
    XNextEvent(display, &event);
    win = findWindow(event.xany.window, &widget);
    switch (event.type) {
    case Expose:
      // redraw the window or widget, ignoring all but the last
      // Expose event for that window
      if (event.xexpose.count == 0 && win) {
	if (widget)
	  widget->redraw();
	else
	  win->redraw();
      }
      break;
    case ConfigureNotify:
      if (win && !widget) {
	if (event.xconfigure.width != win->getWidth() ||
	    event.xconfigure.height != win->getHeight()) {
	  XClearWindow(display, win->getXWindow());
	  win->layout(-1, -1, event.xconfigure.width, event.xconfigure.height);
	}
      }
      break;
    case VisibilityNotify:
      if (event.xvisibility.state == VisibilityUnobscured &&
	  grabWin && win == grabWin->getOverWin())
	XRaiseWindow(display, grabWin->getXWindow());
      break;
    case ButtonPress:
      if (grabWin && win != grabWin)
	goto start;
      if (win) {
	if (widget != win->getKeyWidget()) {
	  if (win->getKeyWidget())
	    win->getKeyWidget()->activate(gFalse);
	  if (widget)
	    widget->activate(gTrue);
	}
	if (widget) {
	  widget->buttonPress(event.xbutton.x, event.xbutton.y,
			      event.xbutton.button - Button1 + 1);
	}
      }
      break;
    case ButtonRelease:
      if (grabWin && win != grabWin)
	goto start;
      if (win && widget) {
	widget->buttonRelease(event.xbutton.x, event.xbutton.y,
			      event.xbutton.button - Button1 + 1);
      }
      break;
    case MotionNotify:
      if (grabWin && win != grabWin)
	goto start;
      if (win && widget) {
	widget->mouseMove(event.xmotion.x, event.xmotion.y);
      }
      break;
    case KeyPress:
      if (grabWin && win != grabWin)
	goto start;
      if (win) {
	n = XLookupString((XKeyEvent *)&event, s, sizeof(s)-1,
			  &key, NULL);
	s[n] = '\0';
	win->keyPress(key, s, n);
      }
      break;
    case PropertyNotify:
      if (grabWin && win != grabWin)
	goto start;
      if (win)
	win->propChange(event.xproperty.atom);
      break;
    default:
      break;
    }
  }
}
