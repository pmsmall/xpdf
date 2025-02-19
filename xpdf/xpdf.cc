//========================================================================
//
// xpdf.cc
//
//========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <X11/cursorfont.h>
#include <parseargs.h>
#include <cover.h>
#include <LTKAll.h>
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

static ArgDesc argDesc[] = {
  {"-cmd",  argFlag, &printCommands, 0,
   "print commands as they're executed"},
  {"-err",  argFlag, &errorsToTTY,   0,
   "send error messages to /dev/tty instead of stderr"},
  {"-h",    argFlag, &printHelp,     0,
   "print usage information"},
  {"-help", argFlag, &printHelp,     0,
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

static Catalog *catalog;
static XOutputDev *out;

static LTKApp *app;
static LTKWindow *win;
static LTKScrollingCanvas *canvas;
static LTKScrollbar *hScrollbar, *vScrollbar;
static LTKTextIn *pageNumText;
static LTKLabel *numPagesLabel;
static LTKWindow *aboutWin;

static int page;
static int zoom;
static int rotate;
static Boolean quit;

int main(int argc, char *argv[]) {
  FILE *f;
  FileStream *str;
  Object catObj;
  Object obj;
  Boolean ok;
  char s[20];

  // init coverage module
  coverInit(200);

  // parse args
  ok = parseArgs(argDesc, &argc, argv);

  // init error file
  errorInit();

  // print banner
  fprintf(errFile, "xpdf version %s\n", xpdfVersion);
  fprintf(errFile, "%s\n", xpdfCopyright);

  // create LTKApp
  app = new LTKApp("xpdf", opts, &argc, argv);
  if (!ok || printHelp || argc != 2) {
    printUsage("xpdf", "<PDF-file>", argDesc);
    exit(1);
  }

  // open PDF file and create stream
  if (!(f = fopen(argv[1], "r"))) {
    error(0, "Couldn't open file '%s'\n", argv[1]);
    exit(1);
  }
  obj.initNull();
  str = new FileStream(f, 0, -1, &obj);

  // check header
  str->checkHeader();

  // read xref table
  xref = new XRef(str);
  delete str;
  if (!xref->isOk()) {
    error(0, "Couldn't read xref table\n");
    fclose(f);
    delete xref;
    exit(1);
  }

  // read catalog
  catalog = new Catalog(xref->getCatalog(&catObj));
  catObj.free();
  if (!catalog->isOk()) {
    error(0, "Couldn't read page catalog\n");
    fclose(f);
    delete xref;
    delete catalog;
    exit(1);
  }

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

  // create output device
  out = new XOutputDev(win);

  // display first page
  page = -99;
  zoom = -99;
  rotate = -99;
  displayPage(1, defZoom, 0);

  // event loop
  quit = false;
  do {
    app->doEvent(true);
  } while (!quit);

  // free stuff
  fclose(f);
  delete out;
  delete win;
  if (aboutWin)
    delete aboutWin;
  delete app;
  delete xref;
  delete catalog;

  // check for memory leaks
  Object::memCheck(errFile);

  // print coverage info
  coverDump(errFile);
}

static void displayPage(int page1, int zoom1, int rotate1) {
  char s[20];

  win->setCursor(XC_watch);

  page = page1;
  sprintf(s, "%d", page);
  pageNumText->setText(s);

  zoom = zoom1;

  rotate = rotate1;

  if (printCommands)
    printf("***** page %d *****\n", page);
  catalog->getPage(page)->display(out, zoomDPI[zoom - minZoom], rotate);
  layoutCanvasCbk(canvas);

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
    XBell(win->getDisplay(), 0);
}

static void prevPageCbk(LTKButton *button, int n, Boolean on) {
  if (page > 1)
    displayPage(page - 1, zoom, rotate);
  else
    XBell(win->getDisplay(), 0);
}

static void pageNumCbk(LTKTextIn *textIn, int n, String *text) {
  int page1;
  char s[20];

  page1 = atoi(text->getCString());
  if (page1 >= 1 && page <= catalog->getNumPages()) {
    displayPage(page1, zoom, rotate);
  } else {
    XBell(win->getDisplay(), 0);
    sprintf(s, "%d", page);
    pageNumText->setText(s);
  }
}

static void zoomInCbk(LTKButton *button, int n, Boolean on) {
  if (zoom < maxZoom)
    displayPage(page, zoom + 1, rotate);
  else
    XBell(win->getDisplay(), 0);
}

static void zoomOutCbk(LTKButton *button, int n, Boolean on) {
  if (zoom > minZoom)
    displayPage(page, zoom - 1, rotate);
  else
    XBell(win->getDisplay(), 0);
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
  if (!aboutWin) {
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
  XSync(scrollbar->getDisplay(), False);
}

static void scrollHorizCbk(LTKScrollbar *scrollbar, int n, int val) {
  canvas->scroll(val, vScrollbar->getPos());
  XSync(scrollbar->getDisplay(), False);
}

static void closeAboutCbk(LTKButton *button, int n, Boolean on) {
  delete aboutWin;
  aboutWin = NULL;
}
