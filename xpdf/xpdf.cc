//========================================================================
//
// xpdf.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/cursorfont.h>
#include <parseargs.h>
#include <cover.h>
#include <LTKAll.h>
#include <String.h>
#include "Object.h"
#include "Stream.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "XOutputDev.h"
#include "Flags.h"
#include "Error.h"
#include "config.h"

#define remoteCmdLength 256

static void killApp();
static void loadFile(String *fileName1);
static void displayPage(int page1, int zoom1, int rotate1);
static void keyPress(LTKWindow *win, KeySym key, char *s, int n);
static void nextPageCbk(LTKButton *button, int n, Boolean on);
static void prevPageCbk(LTKButton *button, int n, Boolean on);
static void pageNumCbk(LTKTextIn *textIn, int n, String *text);
static void zoomInCbk(LTKButton *button, int n, Boolean on);
static void zoomOutCbk(LTKButton *button, int n, Boolean on);
static void rotateCWCbk(LTKButton *button, int n, Boolean on);
static void rotateCCWCbk(LTKButton *button, int n, Boolean on);
static void aboutCbk(LTKButton *button, int n, Boolean on);
static void quitCbk(LTKButton *button, int n, Boolean on);
static void layoutCanvasCbk(LTKScrollingCanvas *canvas);
static void scrollVertCbk(LTKScrollbar *scrollbar, int n, int val);
static void scrollHorizCbk(LTKScrollbar *scrollbar, int n, int val);
static void propChangeCbk(LTKWindow *win, Atom atom);
static void closeAboutCbk(LTKButton *button, int n, Boolean on);

#include "leftArrow.xbm"
#include "rightArrow.xbm"
#include "zoomIn.xbm"
#include "zoomOut.xbm"
#include "rotateCW.xbm"
#include "rotateCCW.xbm"
#include "about.xbm"
#include "xpdf.ltk.h"

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
  {NULL}
};

Boolean printCommands = false;
Boolean printHelp = false;
static char remoteName[100] = "xpdf_";
static Boolean doRemoteRaise = false;
static Boolean doRemoteQuit = false;

static ArgDesc argDesc[] = {
  {"-err",    argFlag,   &errorsToTTY,   0,
   "send error messages to /dev/tty instead of stderr"},
  {"-remote", argString, remoteName + 5, sizeof(remoteName) - 5,
   "start/contact xpdf remote server with specified name"},
  {"-raise",  argFlag,   &doRemoteRaise, 0,
   "raise xpdf remote server window (with -remote only)"},
  {"-quit",   argFlag,   &doRemoteQuit,  0,
   "kill xpdf remote server (with -remote only)"},
  {"-cmd",    argFlag,   &printCommands, 0,
   "print commands as they're executed"},
  {"-h",      argFlag,   &printHelp,     0,
   "print usage information"},
  {"-help",   argFlag,   &printHelp,     0,
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
#define defZoom 1

static String *fileName;
static FILE *file;
static Catalog *catalog;
static XOutputDev *out;

static int page;
static int zoom;
static int rotate;
static Boolean quit;

static LTKApp *app;
static Display *display;
static LTKWindow *win;
static LTKScrollingCanvas *canvas;
static LTKScrollbar *hScrollbar, *vScrollbar;
static LTKTextIn *pageNumText;
static LTKLabel *numPagesLabel;
static LTKWindow *aboutWin;
static Atom remoteAtom;

int main(int argc, char *argv[]) {
  Window xwin;
  char cmd[remoteCmdLength];
  String *name;
  int pg;
  Boolean ok;
  char s[20];

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
    name = new String(argv[1]);
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
			PropModeReplace, (uchar *)cmd, strlen(cmd) + 1);
      } else if (doRemoteRaise) {
	XChangeProperty(display, xwin, remoteAtom, remoteAtom, 8,
			PropModeReplace, (uchar *)"r", 2);
      } else if (doRemoteQuit) {
	XChangeProperty(display, xwin, remoteAtom, remoteAtom, 8,
			PropModeReplace, (uchar *)"q", 2);
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
  loadFile(name);

  // create window
  win = makeWindow(app);
  canvas = (LTKScrollingCanvas *)win->findWidget("canvas");
  hScrollbar = (LTKScrollbar *)win->findWidget("hScrollbar");
  vScrollbar = (LTKScrollbar *)win->findWidget("vScrollbar");
  pageNumText = (LTKTextIn *)win->findWidget("pageNum");
  numPagesLabel = (LTKLabel *)win->findWidget("numPages");
  sprintf(s, "of %d", catalog->getNumPages());
  numPagesLabel->setText(s);
  win->layout((catalog->getPage(1)->getWidth() *
	       zoomDPI[defZoom - minZoom]) / 72 + 28, 680);
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
  zoom = -99;
  rotate = -99;
  displayPage(pg, defZoom, 0);

  // event loop
  quit = false;
  do {
    app->doEvent(true);
  } while (!quit);

  // free stuff
  killApp();
  delete catalog;
  delete xref;
  fclose(file);
  delete fileName;

  // check for memory leaks
  Object::memCheck(errFile);

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

static void loadFile(String *fileName1) {
  FileStream *str;
  Object catObj;
  Object obj;
  char s[20];

  if (win) {
    win->setCursor(XC_watch);
    XFlush(display);
  }

  // free old objects
  if (fileName) {
    delete catalog;
    delete xref;
    fclose(file);
    delete fileName;
  }
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

  // read catalog
  catalog = new Catalog(xref->getCatalog(&catObj));
  catObj.free();
  if (!catalog->isOk()) {
    error(0, "Couldn't read page catalog");
    goto err3;
  }

  // nothing displayed yet
  page = -99;

  if (win) {
    sprintf(s, "of %d", catalog->getNumPages());
    numPagesLabel->setText(s);
    win->setCursor(XC_top_left_arrow);
  }
  return;

 err3:
  delete catalog;
 err2:
  delete xref;
  fclose(file);
 err1:
  delete fileName;
  delete app;
  exit(1);
}

static void displayPage(int page1, int zoom1, int rotate1) {
  char s[20];

  if (win) {
    win->setCursor(XC_watch);
    XFlush(display);
  }

  page = page1;
  zoom = zoom1;
  rotate = rotate1;

  if (printCommands)
    printf("***** page %d *****\n", page);
  catalog->getPage(page)->display(out, zoomDPI[zoom - minZoom], rotate);
  layoutCanvasCbk(canvas);

  sprintf(s, "%d", page);
  pageNumText->setText(s);

  win->setCursor(XC_top_left_arrow);
}

static void keyPress(LTKWindow *win, KeySym key, char *s, int n) {
  if (n <= 0)
    return;
  switch (s[0]) {
  case 'n':
    vScrollbar->setPos(0, canvas->getHeight());
    nextPageCbk(NULL, 0, true);
    break;
  case 'p':
    vScrollbar->setPos(0, canvas->getHeight());
    prevPageCbk(NULL, 0, true);
    break;
  case ' ':
    vScrollbar->setPos(vScrollbar->getPos() + canvas->getHeight(),
		       canvas->getHeight());
    canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
    break;
  case '\b':			// bs
  case '\177':			// del
    vScrollbar->setPos(vScrollbar->getPos() - canvas->getHeight(),
		       canvas->getHeight());
    canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
    break;
  case '\014':			// ^L
    displayPage(page, zoom, rotate);
    break;
  case 'q':
    quitCbk(NULL, 0, true);
    break;
  }
}

static void nextPageCbk(LTKButton *button, int n, Boolean on) {
  if (page < catalog->getNumPages())
    displayPage(page + 1, zoom, rotate);
  else
    XBell(display, 0);
}

static void prevPageCbk(LTKButton *button, int n, Boolean on) {
  if (page > 1)
    displayPage(page - 1, zoom, rotate);
  else
    XBell(display, 0);
}

static void pageNumCbk(LTKTextIn *textIn, int n, String *text) {
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

static void zoomInCbk(LTKButton *button, int n, Boolean on) {
  if (zoom < maxZoom)
    displayPage(page, zoom + 1, rotate);
  else
    XBell(display, 0);
}

static void zoomOutCbk(LTKButton *button, int n, Boolean on) {
  if (zoom > minZoom)
    displayPage(page, zoom - 1, rotate);
  else
    XBell(display, 0);
}

static void rotateCWCbk(LTKButton *button, int n, Boolean on) {
  int r;

  r = (rotate == 270) ? 0 : rotate + 90;
  displayPage(page, zoom, r);
}

static void rotateCCWCbk(LTKButton *button, int n, Boolean on) {
  int r;

  r = (rotate == 0) ? 270 : rotate - 90;
  displayPage(page, zoom, r);
}

static void aboutCbk(LTKButton *button, int n, Boolean on) {
  if (aboutWin) {
    XMapRaised(display, aboutWin->getXWindow());
  } else {
    aboutWin = makeAboutWindow(app);
    aboutWin->layout(0, 0);
    aboutWin->map();
  }
}

static void quitCbk(LTKButton *button, int n, Boolean on) {
  quit = true;
}

static void layoutCanvasCbk(LTKScrollingCanvas *canvas) {
  hScrollbar->setLimits(0, canvas->getRealWidth() - 1);
  hScrollbar->setPos(hScrollbar->getPos(), canvas->getWidth());
  hScrollbar->setScrollDelta(16);
  vScrollbar->setLimits(0, canvas->getRealHeight() - 1);
  vScrollbar->setPos(vScrollbar->getPos(), canvas->getHeight());
  vScrollbar->setScrollDelta(16);
  canvas->scroll(hScrollbar->getPos(), vScrollbar->getPos());
}

static void scrollVertCbk(LTKScrollbar *scrollbar, int n, int val) {
  canvas->scroll(hScrollbar->getPos(), val);
  XSync(display, False);
}

static void scrollHorizCbk(LTKScrollbar *scrollbar, int n, int val) {
  canvas->scroll(val, vScrollbar->getPos());
  XSync(display, False);
}

static void propChangeCbk(LTKWindow *win, Atom atom) {
  Window xwin;
  char *cmd;
  Atom type;
  int format;
  ulong size, remain;
  char *p;
  String *newFileName;
  int newPage;

  // get command
  xwin = win->getXWindow();
  if (XGetWindowProperty(display, xwin, remoteAtom,
			 0, remoteCmdLength/4, True, remoteAtom,
			 &type, &format, &size, &remain,
			 (uchar **)&cmd) != Success)
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
    newFileName = new String(p + 1);
    XFree(cmd);
    if (newFileName->cmp(fileName))
      loadFile(newFileName);
    else
      delete newFileName;
    if (newPage != page && newPage >= 1 && newPage <= catalog->getNumPages())
      displayPage(newPage, zoom, rotate);

  // quit
  } else if (cmd[0] == 'q') {
    quit = true;
  }
}

static void closeAboutCbk(LTKButton *button, int n, Boolean on) {
  delete aboutWin;
  aboutWin = NULL;
}
