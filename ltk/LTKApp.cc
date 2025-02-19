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
#include <unistd.h>
#ifdef HAVE_STRINGS_H
// needed by AIX for bzero() declaration for FD_ZERO
#include <strings.h>
#endif
#ifdef HAVE_BSTRING_H
// needed by IRIX for bzero() declaration for FD_ZERO
#include <bstring.h>
#endif
#ifdef HAVE_SYS_SELECT_H
// needed by some systems for fd_set
#include <sys/select.h>
#endif
#ifdef HAVE_SYS_BSDTYPES_H
// needed by some systems for fd_set
#include <sys/bsdtypes.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "gtypes.h"
#include "LTKApp.h"
#include "LTKResources.h"
#include "LTKWindow.h"
#include "LTKMenu.h"
#include "LTKMisc.h"
#include "LTKWidget.h"

#ifdef XlibSpecificationRelease
#if XlibSpecificationRelease < 5
typedef char *XPointer;
#endif
#else
typedef char *XPointer;
#endif

#ifdef VMS
extern "C" int XMultiplexInput(int num_displays,
			       Display *displays[],
			       unsigned long ef_mask,
			       unsigned long timeout,
			       unsigned long options,
			       long *retval_pointer);
#if defined(__DECCXX) && (_VMS_VER < 70000000)
extern "C" int gettimeofday (struct timeval *__tp, void *__tzp);
#endif
#endif

//------------------------------------------------------------------------

#define ltkSingleClickTime 200	// max time from press to release for
				//   a single click (in ms)

#define ltkDoubleClickTime 200	// max time from press to press for a
				//   double click (in ms)

//------------------------------------------------------------------------
// LTKApp
//------------------------------------------------------------------------

LTKApp::LTKApp(char *appName1, XrmOptionDescRec *opts,
	       int *argc, char *argv[]) {
  int numOpts;
  XrmDatabase cmdLineDB;
  GString *displayName;
  int h;

  appName = new GString(appName1);
  windows = NULL;
  for (h = 0; h < ltkWinTabSize; ++h)
    winTab[h] = NULL;
  grabWin = NULL;
  activeMenu = NULL;
  repeatWidget = NULL;
  repeatDelay = 0;
  repeatPeriod = 0;
  firstRepeat = gTrue;
  cmdLineDB = NULL;
  resourceDB = NULL;
  XrmInitialize();
  for (numOpts = 0; opts[numOpts].option; ++numOpts) ;
  ltkGetCmdLineResources(&cmdLineDB, opts, numOpts, appName, argc, argv);
  displayName = ltkGetStringResource(cmdLineDB, appName, "display", "");
  if (!(display = XOpenDisplay(displayName->getCString()))) {
    ltkError("Cannot connect to X server %s\n",
	     XDisplayName(displayName->getCString()));
    exit(1);
  }
  delete displayName;
  screenNum = DefaultScreen(display);
  ltkGetOtherResources(display, cmdLineDB, &resourceDB);
  wmDeleteWinAtom = XInternAtom(display, "WM_DELETE_WINDOW", False);
  pressedBtn = 0;
  killCbk = NULL;
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

int LTKApp::getIntResource(char *inst, int def) {
  return ltkGetIntResource(resourceDB, appName, inst, def);
}

GBool LTKApp::getBoolResource(char *inst, GBool def) {
  return ltkGetBoolResource(resourceDB, appName, inst, def);
}

unsigned long LTKApp::getColorResource(char *inst,
				       char *def1, unsigned long def2,
				       XColor *xcol) {
  return ltkGetColorResource(resourceDB, appName, inst,
			     display, screenNum, def1, def2, xcol);
}

XFontStruct *LTKApp::getFontResource(char *inst, char *def) {
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
  int h;
  LTKWinHash *p1, *p2, *p3;

  for (w1 = NULL, w2 = windows; w2 && w2 != w; w1 = w2, w2 = w2->getNext()) ;
  if (w2 == w) {

    // remove window from window list
    if (w1)
      w1->setNext(w2->getNext());
    else
      windows = w2->getNext();
    w2->setNext(NULL);

    // remove window and widgets from hash table
    for (h = 0; h < ltkWinTabSize; ++h) {
      p1 = NULL;
      p2 = winTab[h];
      while (p2) {
	if (p2->win == w) {
	  p3 = p2;
	  if (p1)
	    p2 = p1->next = p2->next;
	  else
	    p2 = winTab[h] = p2->next;
	  delete p3;
	} else {
	  p1 = p2;
	  p2 = p2->next;
	}
      }
    }

    return w2;
  }
  return NULL;
}

void LTKApp::registerXWindow(Window xwin, LTKWindow *win, LTKWidget *widget) {
  int h;
  LTKWinHash *p;

  h = (int)xwin % ltkWinTabSize;
  p = new LTKWinHash;
  p->xwin = xwin;
  p->win = win;
  p->widget = widget;
  p->next = winTab[h];
  winTab[h] = p;
}

LTKWindow *LTKApp::findWindow(Window xwin, LTKWidget **widget) {
  int h;
  LTKWinHash *p;

  h = (int)xwin % ltkWinTabSize;
  for (p = winTab[h]; p; p = p->next) {
    if (p->xwin == xwin) {
      *widget = p->widget;
      return p->win;
    }
  }
  *widget = NULL;
  return NULL;
}

void LTKApp::setRepeatEvent(LTKWidget *repeatWidget1, int repeatDelay1,
			    int repeatPeriod1) {
  repeatWidget = repeatWidget1;
  repeatDelay = repeatDelay1;
  repeatPeriod = repeatPeriod1;
  firstRepeat = gTrue;
  gettimeofday(&lastRepeat, NULL);
}

void LTKApp::doEvent(GBool wait) {
#ifdef VMS
  long delay, retval;
#else
  fd_set readFDs, writeFDs, exceptFDs;
#endif
  struct timeval curTime, timeout;
  int timeout1;
  XEvent event;
  XEvent event2;
  Window pointerRoot, pointerChild;
  LTKWindow *win;
  LTKWidget *widget;
  LTKMenu *menu;
  KeySym key;
  GString *str;
  char buf[20];
  Atom typeRet;
  int formatRet;
  unsigned long length, left;
  unsigned char *bufPtr;
  GBool click, dblClick;
  int x, y, rx, ry;
  unsigned int mask;
  int n, i;

  while (XPending(display) == 0) {
    if (!wait)
      return;
#ifndef VMS
    FD_ZERO(&readFDs);
    FD_ZERO(&writeFDs);
    FD_ZERO(&exceptFDs);
    n = ConnectionNumber(display);
    FD_SET(n, &readFDs);
#endif
    if (!repeatWidget) {
#ifdef VMS
      n = XMultiplexInput(1, &display, 0, 0, 0, &retval);
#else // VMS
#ifdef SELECT_TAKES_INT
      n = select(n+1, (int *)&readFDs, (int *)&writeFDs, (int *)&exceptFDs,
		 NULL);
#else
      n = select(n+1, &readFDs, &writeFDs, &exceptFDs, NULL);
#endif
#endif // VMS
    } else {
      gettimeofday(&curTime, NULL);
      timeout.tv_sec = curTime.tv_sec - lastRepeat.tv_sec;
      if (curTime.tv_usec < lastRepeat.tv_usec) {
	--timeout.tv_sec;
	timeout.tv_usec = (1000000 + curTime.tv_usec) - lastRepeat.tv_usec;
      } else {
	timeout.tv_usec = curTime.tv_usec - lastRepeat.tv_usec;
      }
      timeout1 = firstRepeat ? repeatDelay : repeatPeriod;
      if (timeout.tv_sec > 0 || timeout.tv_usec > timeout1)
	timeout.tv_usec = 0;
      else
	timeout.tv_usec = timeout1 - timeout.tv_usec;
      timeout.tv_sec = 0;
#ifdef VMS
      if ((delay = timeout.tv_usec / 1000) == 0)
	delay = 1;
      n = XMultiplexInput(1, &display, 0, delay, 0, &retval);
#else // VMS
#ifdef SELECT_TAKES_INT
      n = select(n+1, (int *)&readFDs, (int *)&writeFDs, (int *)&exceptFDs,
		 &timeout);
#else
      n = select(n+1, &readFDs, &writeFDs, &exceptFDs, &timeout);
#endif
#endif // VMS
    }
    if (n == 0 && repeatWidget) {
      repeatWidget->repeatEvent();
      firstRepeat = gFalse;
      gettimeofday(&lastRepeat, NULL);
      return;
    }
  }

  XNextEvent(display, &event);
  win = findWindow(event.xany.window, &widget);
  if (activeMenu && event.xany.window == activeMenu->getXWindow())
    menu = activeMenu;
  else
    menu = NULL;
  switch (event.type) {
  case Expose:
    // redraw the window or widget, ignoring all but the last
    // Expose event for that window
    if (event.xexpose.count == 0) {
      if (widget)
	widget->redraw();
      else if (win)
	win->redrawBackground();
      else if (menu)
	menu->redraw();
    }
    break;
  case GraphicsExpose:
    // redraw the window or widget, ignoring all but the last
    // GraphicsExpose event for that window
    if (event.xgraphicsexpose.count == 0) {
      if (widget)
	widget->redraw();
      else if (win)
	win->redrawBackground();
      else if (menu)
	menu->redraw();
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
    dblClick = event.xbutton.time - buttonPressTime < ltkDoubleClickTime;
    buttonPressTime = event.xbutton.time;
    pressedBtn = event.xbutton.button - Button1 + 1;
    if (menu) {
      menu->buttonPress(event.xbutton.x, event.xbutton.y,
			pressedBtn, dblClick);
    } else if (win && !(grabWin && win != grabWin)) {
      if (event.xbutton.button == Button3) {
	win->postMenu(event.xbutton.x_root, event.xbutton.y_root);
      } else {
	if (widget != win->getKeyWidget()) {
	  if (win->getKeyWidget())
	    win->getKeyWidget()->activate(gFalse);
	  if (widget)
	    widget->activate(gTrue);
	}
	if (widget) {
	  widget->buttonPress(event.xbutton.x, event.xbutton.y,
			      pressedBtn, dblClick);
	}
      }
    }
    break;
  case ButtonRelease:
    pressedBtn = 0;
    click = event.xbutton.time - buttonPressTime < ltkSingleClickTime;
    if (menu) {
      menu->buttonRelease(event.xbutton.x, event.xbutton.y,
			  event.xbutton.button - Button1 + 1, click);
    } else if (win && !(grabWin && win != grabWin) && widget) {
      widget->buttonRelease(event.xbutton.x, event.xbutton.y,
			    event.xbutton.button - Button1 + 1, click);
    }
    break;
  case MotionNotify:
    // this assumes PointerMotionHintMask was selected, so we need
    // to call XQueryPointer
    XQueryPointer(display, event.xany.window, &pointerRoot, &pointerChild,
		  &rx, &ry, &x, &y, &mask);
    if (menu) {
      menu->mouseMove(x, y, pressedBtn);
    } else if (win && !(grabWin && win != grabWin) && widget) {
      widget->mouseMove(x, y, pressedBtn);
    }
    break;
  case KeyPress:
    if (win && !(grabWin && win != grabWin)) {
      n = XLookupString(&event.xkey, buf, sizeof(buf)-1,
			&key, NULL);
      buf[n] = '\0';
      win->keyPress(key, event.xkey.state, buf, n);
    }
    break;
  case SelectionRequest:
    event2.xselection.type = SelectionNotify;
    event2.xselection.display = display;
    event2.xselection.requestor = event.xselectionrequest.requestor;
    event2.xselection.selection = event.xselectionrequest.selection;
    event2.xselection.target = event.xselectionrequest.target;
    event2.xselection.property = None;
    event2.xselection.time = event.xselectionrequest.time;
    if (event.xselectionrequest.target == XA_STRING &&
	win && (str = win->getSelection())) {
      XChangeProperty(display, event.xselectionrequest.requestor,
		      event.xselectionrequest.property, XA_STRING, 8,
		      PropModeReplace,
		      (Guchar *)str->getCString(), str->getLength());
      event2.xselection.property = event.xselectionrequest.property;
    }
    XSendEvent(display, event.xselectionrequest.requestor,
	       False, 0, &event2);
    break;
  case SelectionClear:
    if (win && win->getSelectionWidget()) {
      win->getSelectionWidget()->clearSelection();
    }
    break;
  case SelectionNotify:
    if (win && !(grabWin && win != grabWin) && win->getPasteWidget() &&
	event.xselection.target == XA_STRING &&
	event.xselection.property != None) {
      i = 0;
      str = new GString();
      do {
	if (XGetWindowProperty(display, win->getXWindow(),
			       event.xselection.property,
			       i/4, 256/4, True, XA_STRING,
			       &typeRet, &formatRet, &length, &left,
			       &bufPtr) != Success)
	  break;
	str->append((char *)bufPtr, (int)length);
	XFree((XPointer)bufPtr);
	i += (int)length;
      } while (left > 0);
      win->getPasteWidget()->paste(str);
      delete str;
    }
    break;
  case PropertyNotify:
    if (win && !(grabWin && win != grabWin))
      win->propChange(event.xproperty.atom);
    break;
  case ClientMessage:
    if ((Atom)event.xclient.data.l[0] == wmDeleteWinAtom && win) {
      if (killCbk)
	(*killCbk)(win);
      else
	exit(0);
    }
    break;
  default:
    break;
  }
}
