//========================================================================
//
// xpdf.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <X11/X.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <parseargs.h>
#include <cover.h>
#include <LTKAll.h>
#include <GString.h>
#include "Object.h"
#include "Stream.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "Link.h"
#include "XOutputDev.h"
#include "PSOutput.h"
#include "Flags.h"
#include "Error.h"
#include "config.h"

#ifndef XK_Page_Up
#define XK_Page_Up              0xFF55
#endif
#ifndef XK_Page_Down
#define XK_Page_Down            0xFF56
#endif

#define remoteCmdLength 256

static void killApp();
static GBool loadFile(GString *fileName1);
static void displayPage(int page1, int zoom1, int rotate1);
static void keyPressCbk(LTKWindow *win, KeySym key, char *s, int n);
static void layoutCbk(LTKWindow *win);
static void propChangeCbk(LTKWindow *win, Atom atom);
static void buttonPressCbk(LTKWidget *canvas1, int n,
			   int mx, int my, int button);
static void nextPageCbk(LTKWidget *button, int n, GBool on);
static void prevPageCbk(LTKWidget *button, int n, GBool on);
static void pageNumCbk(LTKWidget *textIn, int n, GString *text);
static void zoomInCbk(LTKWidget *button, int n, GBool on);
static void zoomOutCbk(LTKWidget *button, int n, GBool on);
static void rotateCWCbk(LTKWidget *button, int n, GBool on);
static void rotateCCWCbk(LTKWidget *button, int n, GBool on);
static void postScriptCbk(LTKWidget *button, int n, GBool on);
static void psButtonCbk(LTKWidget *button, int n, GBool on);
static void psKeyPressCbk(LTKWindow *win, KeySym key, char *s, int n);
static void aboutCbk(LTKWidget *button, int n, GBool on);
static void closeAboutCbk(LTKWidget *button, int n, GBool on);
static void aboutKeyPressCbk(LTKWindow *win, KeySym key, char *s, int n);
static void quitCbk(LTKWidget *button, int n, GBool on);
static void scrollVertCbk(LTKWidget *scrollbar, int n, int val);
static void scrollHorizCbk(LTKWidget *scrollbar, int n, int val);

#include "leftArrow.xbm"
#include "rightArrow.xbm"
#include "zoomIn.xbm"
#include "zoomOut.xbm"
#include "rotateCW.xbm"
#include "rotateCCW.xbm"
#include "postscript.xbm"
#include "about.xbm"
#include "xpdf-ltk.h"

static XrmOptionDescRec opts[] = {
  {"-display",         ".display",         XrmoptionSepArg,    NULL},
  {"-foreground",      ".foreground",      XrmoptionSepArg,    NULL},
  {"-fg",              ".foreground",      XrmoptionSepArg,    NULL},
  {"-background",      ".background",      XrmoptionSepArg,    NULL},
  {"-bg",              ".background",      XrmoptionSepArg,    NULL},
  {"-geometry",        ".geometry",        XrmoptionSepArg,    NULL},
  {"-g",               ".geometry",        XrmoptionSepArg,    NULL},
  {"-font",            ".font",            XrmoptionSepArg,    NULL},
  {"-fn",              ".font",            XrmoptionSepArg,    NULL},
  {"-z",               ".initialZoom",     XrmoptionSepArg,    NULL},
  {NULL}
};

GBool printCommands = gFalse;
static GBool printHelp = gFalse;
static char remoteName[100] = "xpdf_";
static GBool doRemoteRaise = gFalse;
static GBool doRemoteQuit = gFalse;

static ArgDesc argDesc[] = {
  {"-err",      argFlag,        &errorsToTTY,   0,
   "send error messages to /dev/tty instead of stderr"},
  {"-z",        argIntDummy,    NULL,           0,
   "initial zoom level (-5..5)"},
  {"-g",        argStringDummy, NULL,           0,
   "initial window geometry"},
  {"-geometry", argStringDummy, NULL,           0,
   "initial window geometry"},
  {"-remote",   argString,      remoteName + 5, sizeof(remoteName) - 5,
   "start/contact xpdf remote server with specified name"},
  {"-raise",    argFlag,        &doRemoteRaise, 0,
   "raise xpdf remote server window (with -remote only)"},
  {"-quit",     argFlag,        &doRemoteQuit,  0,
   "kill xpdf remote server (with -remote only)"},
  {"-rgb",      argInt,         &rgbCubeSize,   0,
   "biggest RGB cube to allocate (default is 5)"},
  {"-cmd",      argFlag,        &printCommands, 0,
   "print commands as they're executed"},
  {"-h",        argFlag,        &printHelp,     0,
   "print usage information"},
  {"-help",     argFlag,        &printHelp,     0,
   "print usage information"},
  {NULL}
};

// zoom factor is 1.2 (similar to DVI magsteps)
#define minZoom -5
#define maxZoom  5
static int zoomDPI[maxZoom - minZoom + 1] = {
  29, 35, 42, 50, 60,
  72,
  86, 104, 124, 149, 179
};
#define defZoom "1"

static GString *fileName;
static FILE *file;
static Catalog *catalog;
static XOutputDev *out;
static Links *links;

static int page;
static int zoom;
static int rotate;
static GBool quit;

static GString *psFileName;
static int psFirstPage, psLastPage;

static LTKApp *app;
static Display *display;
static LTKWindow *win;
static LTKScrollingCanvas *canvas;
static LTKScrollbar *hScrollbar, *vScrollbar;
static LTKTextIn *pageNumText;
static LTKLabel *numPagesLabel;
static LTKWindow *aboutWin;
static LTKWindow *psDialog;
static Atom remoteAtom;

int main(int argc, char *argv[]) {
  Window xwin;
  char cmd[remoteCmdLength];
  GString *name;
  int pg, zoom1;
  int x, y;
  Guint width, height;
  GBool ok;
  char s[20];
  GString *str;

  // init coverage module
  coverInit(200);

  // parse args
  ok = parseArgs(argDesc, &argc, argv);

  // init error file
  errorInit();

  // create LTKApp (and parse X-related args)
  app = new LTKApp("xpdf", opts, &argc, argv);
  display = app->getDisplay();
  win = NULL;

  // check command line
  if (doRemoteRaise)
    ok = ok && remoteName[5] && !doRemoteQuit &&
         (argc == 1 || argc == 2 || argc == 3);
  else if (doRemoteQuit)
    ok = ok && remoteName[5] && argc == 1;
  else
    ok = ok && (argc == 2 || argc == 3);
  if (!ok || printHelp) {
    printUsage("xpdf", "[<PDF-file> [<page>]]", argDesc);
    exit(1);
  }
  if (argc >= 2)
    name = new GString(argv[1]);
  else
    name = NULL;
  if (argc == 3)
    pg = atoi(argv[2]);
  else
    pg = 1;

  // look for already-running remote server
  if (remoteName[5]) {
    remoteAtom = XInternAtom(display, remoteName, False);
    xwin = XGetSelectionOwner(display, remoteAtom);
    if (xwin != None) {
      if (name) {
	sprintf(cmd, "%c %d %.200s", doRemoteRaise ? 'D' : 'd',
		pg, name->getCString());
	XChangeProperty(display, xwin, remoteAtom, remoteAtom, 8,
			PropModeReplace, (Guchar *)cmd, strlen(cmd) + 1);
      } else if (doRemoteRaise) {
	XChangeProperty(display, xwin, remoteAtom, remoteAtom, 8,
			PropModeReplace, (Guchar *)"r", 2);
      } else if (doRemoteQuit) {
	XChangeProperty(display, xwin, remoteAtom, remoteAtom, 8,
			PropModeReplace, (Guchar *)"q", 2);
      }
      delete app;
      exit(0);
    }
    if (!name)
      exit(0);
  } else {
    remoteAtom = None;
  }

  // print banner
  fprintf(errFile, "xpdf version %s\n", xpdfVersion);
  fprintf(errFile, "%s\n", xpdfCopyright);

  // open PDF file
  fileName = NULL;
  psFileName = NULL;
  if (!loadFile(name)) {
    delete app;
    exit(1);
  }

  // create window
  win = makeWindow(app);
  canvas = (LTKScrollingCanvas *)win->findWidget("canvas");
  hScrollbar = (LTKScrollbar *)win->findWidget("hScrollbar");
  vScrollbar = (LTKScrollbar *)win->findWidget("vScrollbar");
  pageNumText = (LTKTextIn *)win->findWidget("pageNum");
  numPagesLabel = (LTKLabel *)win->findWidget("numPages");
  win->setKeyCbk(&keyPressCbk);
  win->setLayoutCbk(&layoutCbk);
  canvas->setButtonPressCbk(&buttonPressCbk);

  // get X resources
  str = app->getStringResource("initialZoom", defZoom);
  zoom1 = atoi(str->getCString());
  delete str;
  x = -1;
  y = -1;
  width = (catalog->getPage(1)->getWidth() *
	   zoomDPI[zoom1 - minZoom]) / 72 + 28;
  if (width > (Guint)app->getDisplayWidth() - 100)
    width = app->getDisplayWidth() - 100;
  height = (catalog->getPage(1)->getHeight() *
	    zoomDPI[zoom1 - minZoom]) / 72 + 56;
  if (height > (Guint)app->getDisplayHeight() - 100)
    height = app->getDisplayHeight() - 100;
  app->getGeometryResource("geometry", &x, &y, &width, &height);

  // finish setting up window
  sprintf(s, "of %d", catalog->getNumPages());
  numPagesLabel->setText(s);
  win->layout(x, y, width, height);
  win->map();
  aboutWin = NULL;

  // set up remote server
  if (remoteAtom != None) {
    win->setPropChangeCbk(&propChangeCbk);
    xwin = win->getXWindow();
    XSetSelectionOwner(display, remoteAtom, xwin, CurrentTime);
  }

  // create output device
  out = new XOutputDev(win);

  // display first page
  if (pg < 1 || pg > catalog->getNumPages())
    pg = 1;
  if (zoom1 < minZoom)
    zoom1 = minZoom;
  else if (zoom1 > maxZoom)
    zoom1 = maxZoom;
  links = NULL;
  displayPage(pg, zoom1, 0);

  // event loop
  quit = gFalse;
  do {
    app->doEvent(gTrue);
  } while (!quit);

  // free stuff
  killApp();
  if (links)
    delete links;
  delete catalog;
  delete xref;
  fclose(file);
  delete fileName;
  delete psFileName;

  // check for memory leaks
  Object::memCheck(errFile);
  gMemReport(errFile);

  // print coverage info
  coverDump(errFile);
}

static void killApp() {
  delete out;
  if (remoteAtom != None)
    XSetSelectionOwner(display, remoteAtom, None, CurrentTime);
  delete win;
  if (aboutWin)
    delete aboutWin;
  delete app;
}

static GBool loadFile(GString *fileName1) {
  Catalog *oldCatalog;
  XRef *oldXref;
  FILE *oldFile;
  GString *oldFileName;
  FileStream *str;
  Object catObj;
  Object obj;
  char s[20];
  char *p;

  // busy cursor
  if (win) {
    win->setCursor(XC_watch);
    XFlush(display);
  }

  // save current document
  oldCatalog = catalog;
  oldXref = xref;
  oldFile = file;
  oldFileName = fileName;

  // no xref yet
  xref = NULL;

  // new file name
  fileName = fileName1;

  // open PDF file and create stream
  if (!(file = fopen(fileName->getCString(), "r"))) {
    error(0, "Couldn't open file '%s'", fileName->getCString());
    goto err1;
  }
  obj.initNull();
  str = new FileStream(file, 0, -1, &obj);

  // check header
  str->checkHeader();

  // read xref table
  xref = new XRef(str);
  delete str;
  if (!xref->isOk()) {
    error(0, "Couldn't read xref table");
    goto err2;
  }
  if (xref->checkEncrypted())
    goto err2;

  // read catalog
  catalog = new Catalog(xref->getCatalog(&catObj));
  catObj.free();
  if (!catalog->isOk()) {
    error(0, "Couldn't read page catalog");
    goto err3;
  }

  // delete old document
  if (oldFileName) {
    delete oldCatalog;
    delete oldXref;
    fclose(oldFile);
    delete oldFileName;
  }

  // nothing displayed yet
  page = -99;

  // init PostScript output params
  if (psFileName)
    delete psFileName;
  p = fileName->getCString() + fileName->getLength() - 4;
  if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF"))
    psFileName = new GString(fileName->getCString(),
			     fileName->getLength() - 4);
  else
    psFileName = fileName->copy();
  psFileName->append(".ps");
  psFirstPage = 1;
  psLastPage = catalog->getNumPages();

  // set up number-of-pages display, back to normal cursor
  if (win) {
    sprintf(s, "of %d", catalog->getNumPages());
    numPagesLabel->setText(s);
    win->setCursor(XC_top_left_arrow);
  }

  // done
  return gTrue;

 err3:
  delete catalog;
 err2:
  delete xref;
  fclose(file);
 err1:
  delete fileName;

  // restore old document
  catalog = oldCatalog;
  xref = oldXref;
  file = oldFile;
  fileName = oldFileName;
  if (win)
    win->setCursor(XC_top_left_arrow);

  return gFalse;
}

static void displayPage(int page1, int zoom1, int rotate1) {
  Page *p;
  char s[20];
  Object obj;

  // busy cursor
  if (win) {
    win->setCursor(XC_watch);
    XFlush(display);
  }

  // new page/zoom/rotate values
  page = page1;
  zoom = zoom1;
  rotate = rotate1;

  // draw the page
  if (printCommands)
    printf("***** page %d *****\n", page);
  p = catalog->getPage(page);
  p->display(out, zoomDPI[zoom - minZoom], rotate);
  layoutCbk(win);

  // update page number display
  sprintf(s, "%d", page);
  pageNumText->setText(s);

  // get links
  if (links)
    delete links;
  links = new Links(p->getAnnots(&obj), catalog);
  obj.free();

  // back to regular cursor
  win->setCursor(XC_top_left_arrow);
}

static void keyPressCbk(LTKWindow *win, KeySym key, char *s, int n) {
  if (n > 0) {
    switch (s[0]) {
    case 'n':
      nextPageCbk(NULL, 0, gTrue);
      break;
    case 'p':
      prevPageCbk(NULL, 0, gTrue);
      break;
    case ' ':
      if (vScrollbar->getPos() >=
	  canvas->getRealHeight() - canvas->getHeight()) {
	nextPageCbk(NULL, 0, gTrue);
      } else {
	vScrollbar->setPos(vScrollbar->getPos() + canvas->getHeight(),
			   canvas->getHeight());
	canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
      }
      break;
    case '\b':			// bs
    case '\177':		// del
      if (vScrollbar->getPos() == 0) {
	prevPageCbk(NULL, 0, gTrue);
      } else {
	vScrollbar->setPos(vScrollbar->getPos() - canvas->getHeight(),
			   canvas->getHeight());
	canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
      }
      break;
    case '\014':		// ^L
      displayPage(page, zoom, rotate);
      break;
    case 'q':
      quitCbk(NULL, 0, gTrue);
      break;
    }
  } else {
    switch (key) {
    case XK_Home:
      hScrollbar->setPos(0, canvas->getWidth());
      vScrollbar->setPos(0, canvas->getHeight());
      canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
      break;
    case XK_End:
      hScrollbar->setPos(canvas->getRealWidth() - canvas->getWidth(),
			 canvas->getWidth());
      vScrollbar->setPos(canvas->getRealHeight() - canvas->getHeight(),
			 canvas->getHeight());
      canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
      break;
    case XK_Page_Up:
      if (vScrollbar->getPos() == 0) {
	prevPageCbk(NULL, 0, gTrue);
      } else {
	vScrollbar->setPos(vScrollbar->getPos() - canvas->getHeight(),
			   canvas->getHeight());
	canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
      }
      break;
    case XK_Page_Down:
      if (vScrollbar->getPos() >=
	  canvas->getRealHeight() - canvas->getHeight()) {
	nextPageCbk(NULL, 0, gTrue);
      } else {
	vScrollbar->setPos(vScrollbar->getPos() + canvas->getHeight(),
			   canvas->getHeight());
	canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
      }
      break;
    case XK_Left:
      hScrollbar->setPos(hScrollbar->getPos() - 16, canvas->getWidth());
      canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
      break;
    case XK_Right:
      hScrollbar->setPos(hScrollbar->getPos() + 16, canvas->getWidth());
      canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
      break;
    case XK_Up:
      vScrollbar->setPos(vScrollbar->getPos() - 16, canvas->getHeight());
      canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
      break;
    case XK_Down:
      vScrollbar->setPos(vScrollbar->getPos() + 16, canvas->getHeight());
      canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
      break;
    }
  }
}

static void layoutCbk(LTKWindow *win) {
  hScrollbar->setLimits(0, canvas->getRealWidth() - 1);
  hScrollbar->setPos(hScrollbar->getPos(), canvas->getWidth());
  hScrollbar->setScrollDelta(16);
  vScrollbar->setLimits(0, canvas->getRealHeight() - 1);
  vScrollbar->setPos(vScrollbar->getPos(), canvas->getHeight());
  vScrollbar->setScrollDelta(16);
  canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
}

static void propChangeCbk(LTKWindow *win, Atom atom) {
  Window xwin;
  char *cmd;
  Atom type;
  int format;
  Gulong size, remain;
  char *p;
  GString *newFileName;
  int newPage;

  // get command
  xwin = win->getXWindow();
  if (XGetWindowProperty(display, xwin, remoteAtom,
			 0, remoteCmdLength/4, True, remoteAtom,
			 &type, &format, &size, &remain,
			 (Guchar **)&cmd) != Success)
    return;
  if (size == 0)
    return;

  // raise window
  if (cmd[0] == 'D' || cmd[0] == 'r'){
    XMapRaised(display, xwin);
    XFlush(display);
  }

  // display file / page
  if (cmd[0] == 'd' || cmd[0] == 'D') {
    p = cmd + 2;
    newPage = atoi(p);
    if (!(p = strchr(p, ' ')))
      return;
    newFileName = new GString(p + 1);
    XFree(cmd);
    if (newFileName->cmp(fileName)) {
      if (!loadFile(newFileName))
	return;
    } else {
      delete newFileName;
    }
    if (newPage != page && newPage >= 1 && newPage <= catalog->getNumPages())
      displayPage(newPage, zoom, rotate);

  // quit
  } else if (cmd[0] == 'q') {
    quit = gTrue;
  }
}

static void buttonPressCbk(LTKWidget *canvas1, int n,
			   int mx, int my, int button) {
  Link *link;
  LinkAction *action = NULL;
  LinkGoto *dest;
  LinkURI *uri;
  double x, y;
  int dx, dy;

  if (button == 1) {
    out->cvtDevToUser(mx, my, &x, &y);
    if ((link = links->find((int)x, (int)y)) &&
	(action = link->getAction())) {

      // goto action
      if (action->getKind() == actionGoto) {
	dest = (LinkGoto *)action;
	if (dest->isRemote()) {
	  if (!loadFile(dest->getFileName()->copy()))
	    return;
	}
	if (dest->getPageNum() != page)
	  displayPage(dest->getPageNum(), zoom, rotate);
	switch (dest->getDestKind()) {
	case gotoXYZ:
	  out->cvtUserToDev(dest->getLeft(), dest->getTop(), &dx, &dy);
	  if (dest->getChangeLeft() || dest->getChangeTop()) {
	    if (dest->getChangeLeft())
	      hScrollbar->setPos(dx, canvas->getWidth());
	    if (dest->getChangeTop())
	      vScrollbar->setPos(dy, canvas->getHeight());
	    canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
	  }
	  //~ what is the zoom parameter?
	  break;
	case gotoFit:
	case gotoFitB:
	  //~ do fit
	  hScrollbar->setPos(0, canvas->getWidth());
	  vScrollbar->setPos(0, canvas->getHeight());
	  canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
	  break;
	case gotoFitH:
	case gotoFitBH:
	  //~ do fit
	  out->cvtUserToDev(0, dest->getTop(), &dx, &dy);
	  hScrollbar->setPos(0, canvas->getWidth());
	  vScrollbar->setPos(dy, canvas->getHeight());
	  canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
	  break;
	case gotoFitV:
	case gotoFitBV:
	  //~ do fit
	  out->cvtUserToDev(dest->getLeft(), 0, &dx, &dy);
	  hScrollbar->setPos(dx, canvas->getWidth());
	  vScrollbar->setPos(0, canvas->getHeight());
	  canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
	  break;
	case gotoFitR:
	  //~ do fit
	  out->cvtUserToDev(dest->getLeft(), dest->getTop(), &dx, &dy);
	  hScrollbar->setPos(dx, canvas->getWidth());
	  vScrollbar->setPos(dy, canvas->getHeight());
	  canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
	  break;
	}

      // URI action
      } else if (action->getKind() == actionURI) {
	uri = (LinkURI *)action;
	fprintf(errFile, "URI: %s\n", uri->getURI()->getCString());

      // unknown action type
      } else if (action->getKind() == actionUnknown) {
	error(0, "Unknown link action type: '%s'",
	      ((LinkUnknown *)action)->getAction()->getCString());
      }
    }
  }
}

static void nextPageCbk(LTKWidget *button, int n, GBool on) {
  if (page < catalog->getNumPages()) {
    vScrollbar->setPos(0, canvas->getHeight());
    displayPage(page + 1, zoom, rotate);
  } else {
    XBell(display, 0);
  }
}

static void prevPageCbk(LTKWidget *button, int n, GBool on) {
  if (page > 1) {
    vScrollbar->setPos(0, canvas->getHeight());
    displayPage(page - 1, zoom, rotate);
  } else {
    XBell(display, 0);
  }
}

static void pageNumCbk(LTKWidget *textIn, int n, GString *text) {
  int page1;
  char s[20];

  page1 = atoi(text->getCString());
  if (page1 >= 1 && page1 <= catalog->getNumPages()) {
    displayPage(page1, zoom, rotate);
  } else {
    XBell(display, 0);
    sprintf(s, "%d", page);
    pageNumText->setText(s);
  }
}

static void zoomInCbk(LTKWidget *button, int n, GBool on) {
  if (zoom < maxZoom)
    displayPage(page, zoom + 1, rotate);
  else
    XBell(display, 0);
}

static void zoomOutCbk(LTKWidget *button, int n, GBool on) {
  if (zoom > minZoom)
    displayPage(page, zoom - 1, rotate);
  else
    XBell(display, 0);
}

static void rotateCWCbk(LTKWidget *button, int n, GBool on) {
  int r;

  r = (rotate == 270) ? 0 : rotate + 90;
  displayPage(page, zoom, r);
}

static void rotateCCWCbk(LTKWidget *button, int n, GBool on) {
  int r;

  r = (rotate == 0) ? 270 : rotate - 90;
  displayPage(page, zoom, r);
}

static void postScriptCbk(LTKWidget *button, int n, GBool on) {
  LTKTextIn *widget;
  char s[20];

  psDialog = makePostScriptDialog(app);
  psDialog->setKeyCbk(&psKeyPressCbk);
  sprintf(s, "%d", psFirstPage);
  widget = (LTKTextIn *)psDialog->findWidget("firstPage");
  widget->setText(s);
  sprintf(s, "%d", psLastPage);
  widget = (LTKTextIn *)psDialog->findWidget("lastPage");
  widget->setText(s);
  widget = (LTKTextIn *)psDialog->findWidget("fileName");
  widget->setText(psFileName->getCString());
  psDialog->layoutDialog(win, -1, -1);
  psDialog->map();
}

static void psButtonCbk(LTKWidget *button, int n, GBool on) {
  PSOutput *psOut;
  LTKTextIn *widget;
  int pg;

  // "Ok" button
  if (n == 1) {
    // extract params and close the dialog
    widget = (LTKTextIn *)psDialog->findWidget("firstPage");
    psFirstPage = atoi(widget->getText());
    if (psFirstPage < 1)
      psFirstPage = 1;
    widget = (LTKTextIn *)psDialog->findWidget("lastPage");
    psLastPage = atoi(widget->getText());
    if (psLastPage < psFirstPage)
      psLastPage = psFirstPage;
    else if (psLastPage > catalog->getNumPages())
      psLastPage = catalog->getNumPages();
    widget = (LTKTextIn *)psDialog->findWidget("fileName");
    if (psFileName)
      delete psFileName;
    psFileName = new GString(widget->getText());

    // do the PostScript output
    psDialog->setCursor(XC_watch);
    win->setCursor(XC_watch);
    XFlush(display);
    psOut = new PSOutput(psFileName->getCString(), catalog,
			 psFirstPage, psLastPage);
    if (psOut->isOk()) {
      for (pg = psFirstPage; pg <= psLastPage; ++pg) {
	if (printCommands)
	  printf("***** page %d [PostScript] *****\n", page);
	catalog->getPage(pg)->genPostScript(psOut, zoomDPI[zoom - minZoom],
					    rotate);
      }
      psOut->trailer();
    }
    delete psOut;

    delete psDialog;
    win->setCursor(XC_top_left_arrow);

  // "Cancel" button
  } else {
    delete psDialog;
  }
}

static void psKeyPressCbk(LTKWindow *win, KeySym key, char *s, int n) {
  if (n > 0 && (s[0] == '\n' || s[0] == '\r'))
    psButtonCbk(NULL, 1, gTrue);
}

static void aboutCbk(LTKWidget *button, int n, GBool on) {
  if (aboutWin) {
    XMapRaised(display, aboutWin->getXWindow());
  } else {
    aboutWin = makeAboutWindow(app);
    aboutWin->setKeyCbk(&aboutKeyPressCbk);
    aboutWin->layout(-1, -1, -1, -1);
    aboutWin->map();
  }
}

static void closeAboutCbk(LTKWidget *button, int n, GBool on) {
  delete aboutWin;
  aboutWin = NULL;
}

static void aboutKeyPressCbk(LTKWindow *win, KeySym key, char *s, int n) {
  if (n > 0 && (s[0] == '\n' || s[0] == '\r'))
    closeAboutCbk(NULL, 0, gTrue);
}

static void quitCbk(LTKWidget *button, int n, GBool on) {
  quit = gTrue;
}

static void scrollVertCbk(LTKWidget *scrollbar, int n, int val) {
  canvas->scroll(hScrollbar->getPos(), val);
  XSync(display, False);
}

static void scrollHorizCbk(LTKWidget *scrollbar, int n, int val) {
  canvas->scroll(val, vScrollbar->getPos());
  XSync(display, False);
}
