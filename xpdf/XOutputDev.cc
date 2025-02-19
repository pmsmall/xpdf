//========================================================================
//
// XOutputDev.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <gmem.h>
#include <GString.h>
#include <LTKWindow.h>
#include <LTKScrollingCanvas.h>
#include "Object.h"
#include "Stream.h"
#include "GfxState.h"
#include "GfxFont.h"
#include "Error.h"
#include "Flags.h"
#include "XOutputDev.h"

#include "XOutputFontInfo.h"

//------------------------------------------------------------------------
// Constants and macros
//------------------------------------------------------------------------

#define numTmpSubpaths 16	// number of elements in temporary arrays

#define xoutRound(x) ((int)(x + 0.5))

//------------------------------------------------------------------------
// Misc types
//------------------------------------------------------------------------

struct BoundingRect {
  short xMin, xMax;		// min/max x values
  short yMin, yMax;		// min/max y values
};

struct RGBColor {
  int r, g, b;
};

//------------------------------------------------------------------------
// Command line options
//------------------------------------------------------------------------

int rgbCubeSize = defaultRGBCube;

//------------------------------------------------------------------------
// Font map
//------------------------------------------------------------------------

struct FontMapEntry {
  char *pdfFont;
  char *xFont;
  char **encoding;
  int encodingSize;
};

static FontMapEntry fontMap[] = {
  {"Courier",               "-*-courier-medium-r-*-*-%s-*-*-*-*-*-iso8859-1",
   isoEncoding,          isoEncodingSize},
  {"Courier-Bold",          "-*-courier-bold-r-*-*-%s-*-*-*-*-*-iso8859-1",
   isoEncoding,          isoEncodingSize},
  {"Courier-BoldOblique",   "-*-courier-bold-o-*-*-%s-*-*-*-*-*-iso8859-1",
   isoEncoding,          isoEncodingSize},
  {"Courier-Oblique",       "-*-courier-medium-o-*-*-%s-*-*-*-*-*-iso8859-1",
   isoEncoding,          isoEncodingSize},
  {"Helvetica",             "-*-helvetica-medium-r-*-*-%s-*-*-*-*-*-iso8859-1",
   isoEncoding,          isoEncodingSize},
  {"Helvetica-Bold",        "-*-helvetica-bold-r-*-*-%s-*-*-*-*-*-iso8859-1",
   isoEncoding,          isoEncodingSize},
  {"Helvetica-BoldOblique", "-*-helvetica-bold-o-*-*-%s-*-*-*-*-*-iso8859-1",
   isoEncoding,          isoEncodingSize},
  {"Helvetica-Oblique",     "-*-helvetica-medium-o-*-*-%s-*-*-*-*-*-iso8859-1",
   isoEncoding,          isoEncodingSize},
  {"Symbol",                "-*-symbol-medium-r-*-*-%s-*-*-*-*-*-adobe-fontspecific",
   symbolEncoding,       symbolEncodingSize},
  {"Times-Bold",            "-*-times-bold-r-*-*-%s-*-*-*-*-*-iso8859-1",
   isoEncoding,          isoEncodingSize},
  {"Times-BoldItalic",      "-*-times-bold-i-*-*-%s-*-*-*-*-*-iso8859-1",
   isoEncoding,          isoEncodingSize},
  {"Times-Italic",          "-*-times-medium-i-*-*-%s-*-*-*-*-*-iso8859-1",
   isoEncoding,          isoEncodingSize},
  {"Times-Roman",           "-*-times-medium-r-*-*-%s-*-*-*-*-*-iso8859-1",
   isoEncoding,          isoEncodingSize},
  {"ZapfDingbats",          "-*-zapfdingbats-medium-r-*-*-%s-*-*-*-*-*-itc-fontspecific",
   zapfDingbatsEncoding, zapfDingbatsEncodingSize},
  {NULL}
};

//------------------------------------------------------------------------
// Font substitutions
//------------------------------------------------------------------------

struct FontSubst {
  char *xFont;
  double mWidth;
};

// index: {symbolic:12, fixed:8, serif:4, sans-serif:0} + bold*2 + italic
static FontSubst fontSubst[16] = {
  {"-*-helvetica-medium-r-*-*-%s-*-*-*-*-*-iso8859-1",       0.833},
  {"-*-helvetica-medium-o-*-*-%s-*-*-*-*-*-iso8859-1",       0.833},
  {"-*-helvetica-bold-r-*-*-%s-*-*-*-*-*-iso8859-1",         0.889},
  {"-*-helvetica-bold-o-*-*-%s-*-*-*-*-*-iso8859-1",         0.889},
  {"-*-times-medium-r-*-*-%s-*-*-*-*-*-iso8859-1",           0.788},
  {"-*-times-medium-i-*-*-%s-*-*-*-*-*-iso8859-1",           0.722},
  {"-*-times-bold-r-*-*-%s-*-*-*-*-*-iso8859-1",             0.833},
  {"-*-times-bold-i-*-*-%s-*-*-*-*-*-iso8859-1",             0.778},
  {"-*-courier-medium-r-*-*-%s-*-*-*-*-*-iso8859-1",         0.600},
  {"-*-courier-medium-o-*-*-%s-*-*-*-*-*-iso8859-1",         0.600},
  {"-*-courier-bold-r-*-*-%s-*-*-*-*-*-iso8859-1",           0.600},
  {"-*-courier-bold-o-*-*-%s-*-*-*-*-*-iso8859-1",           0.600},
  {"-*-symbol-medium-r-*-*-%s-*-*-*-*-*-adobe-fontspecific", 0.576},
  {"-*-symbol-medium-r-*-*-%s-*-*-*-*-*-adobe-fontspecific", 0.576},
  {"-*-symbol-medium-r-*-*-%s-*-*-*-*-*-adobe-fontspecific", 0.576},
  {"-*-symbol-medium-r-*-*-%s-*-*-*-*-*-adobe-fontspecific", 0.576}
};

//------------------------------------------------------------------------
// Constructed characters
//------------------------------------------------------------------------

#define lastRegularChar 0x0ff
#define firstSubstChar  0x100
#define lastSubstChar   0x104
#define firstConstrChar 0x105
#define lastConstrChar  0x106
#define firstMultiChar  0x107
#define lastMultiChar   0x10d

// substituted chars
static Guchar substChars[] = {
  0x27,				// 100: quotesingle --> quoteright
  0x2d,				// 101: emdash --> hyphen
  0x2d,				// 102: minus --> hyphen
  0x2f,				// 103: fraction --> slash
  0xb0,				// 104: ring --> degree
};

// constructed chars
// 105: bullet
// 106: trademark

// built-up chars
static char *multiChars[] = {
  "fi",				// 107: fi
  "fl",				// 108: fl
  "OE",				// 109: OE
  "oe",				// 10a: oe
  "...",			// 10b: ellipsis
  "``",				// 10c: quotedblleft
  "''"				// 10d: quotedblright
};

// ignored chars
// 10c: Lslash
// 10d: Scaron
// 10e: Zcaron
// 10f: Ydieresis
// 110: breve
// 111: caron
// 112: circumflex
// 113: dagger
// 114: daggerdbl
// 115: dotaccent
// 116: dotlessi
// 117: florin
// 118: grave
// 119: guilsinglleft
// 11a: guilsinglright
// 11b: hungarumlaut
// 11c: lslash
// 11d: ogonek
// 11e: perthousand
// 11f: quotedblbase
// 120: quotesinglbase
// 121: scaron
// 122: tilde
// 123: zcaron

//------------------------------------------------------------------------
// XOutputFont
//------------------------------------------------------------------------

// Note: if real font is substantially narrower than substituted
// font, the size is reduced accordingly.
XOutputFont::XOutputFont(GfxFont *gfxFont, double m11, double m12,
			 double m21, double m22, Display *display1) {
  GString *pdfFont;
  FontMapEntry *p;
  char **encoding;
  int encodingSize;
  char *fontNameFmt;
  char fontName[200], fontSize[100];
  GBool rotated;
  double size;
  int startSize, sz;
  int index;
  int code, code2;
  double w1, w2;
  char *charName;

  // init
  tag = gfxFont->getTag()->copy();
  mat11 = m11;
  mat12 = m12;
  mat21 = m21;
  mat22 = m22;
  display = display1;
  xFont = NULL;
  revMap = NULL;

  // construct X font name
  pdfFont = gfxFont->getName();
  if (pdfFont) {
    for (p = fontMap; p->pdfFont; ++p) {
      if (!pdfFont->cmp(p->pdfFont))
	break;
    }
  } else {
    p = NULL;
  }
  if (p && p->pdfFont) {
    fontNameFmt = p->xFont;
    encoding = p->encoding;
    encodingSize = p->encodingSize;
  } else {
    encoding = isoEncoding;
    encodingSize = isoEncodingSize;
//~ Some non-symbolic fonts are tagged as symbolic.
//~    if (gfxFont->isSymbolic()) {
//~      index = 12;
//~      encoding = symbolEncoding;
//~      encodingSize = symbolEncodingSize;
//~    } else
    if (gfxFont->isFixedWidth()) {
      index = 8;
    } else if (gfxFont->isSerif()) {
      index = 4;
    } else {
      index = 0;
    }
    if (gfxFont->isBold())
      index += 2;
    if (gfxFont->isItalic())
      index += 1;
    if (!gfxFont->isSymbolic()) {
      w1 = gfxFont->getWidth('m');
      w2 = fontSubst[index].mWidth;
      if (w1 > 0.01 && w1 < 0.9 * w2) {
	w1 /= 0.9 * w2;
	mat11 *= w1;
	mat12 *= w1;
	mat21 *= w1;
	mat22 *= w1;
      }
    }
    fontNameFmt = fontSubst[index].xFont;
  }

  // compute size, normalize matrix
  size = sqrt(mat21*mat21 + mat22*mat22);
  mat11 = mat11 / size;
  mat12 = -mat12 / size;
  mat21 = mat21 / size;
  mat22 = -mat22 / size;
  startSize = (int)size;

  // try to get a rotated font?
  rotated = !(mat11 > 0 && mat22 > 0 && fabs(mat11/mat22 - 1) < 0.2 &&
	      fabs(mat12) < 0.01 && fabs(mat21) < 0.01);

  // open X font -- if font is not found (which means the server can't
  // scale fonts), try progressively smaller and then larger sizes
  //~ This does a linear search -- it should get a list of fonts from
  //~ the server and pick the closest.
  if (rotated)
    sprintf(fontSize, "[%s%0.2f %s%0.2f %s%0.2f %s%0.2f]",
	    mat11<0 ? "~" : "", fabs(mat11 * startSize),
	    mat12<0 ? "~" : "", fabs(mat12 * startSize),
	    mat21<0 ? "~" : "", fabs(mat21 * startSize),
	    mat22<0 ? "~" : "", fabs(mat22 * startSize));
  else
    sprintf(fontSize, "%d", startSize);
  sprintf(fontName, fontNameFmt, fontSize);
  xFont = XLoadQueryFont(display, fontName);
  if (!xFont) {
    for (sz = startSize; sz >= 1; --sz) {
      sprintf(fontSize, "%d", sz);
      sprintf(fontName, fontNameFmt, fontSize);
      if ((xFont = XLoadQueryFont(display, fontName)))
	break;
    }
    if (!xFont) {
      for (sz = startSize + 1; sz < 10; ++sz) {
	sprintf(fontSize, "%d", sz);
	sprintf(fontName, fontNameFmt, fontSize);
	if ((xFont = XLoadQueryFont(display, fontName)))
	  break;
      }
      if (!xFont) {
	sprintf(fontSize, "%d", startSize);
	sprintf(fontName, fontNameFmt, fontSize);
	error(0, "Failed to open font: '%s'", fontName);
	return;
      }
    }
  }

  // construct forward and reverse map
  revMap = (Guchar *)gmalloc(encodingSize * sizeof(Guchar));
  for (code = 0; code < 256; ++code) {
    charName = gfxFont->getCharName(code);
    if (charName) {
      code2 = gfxFont->lookupCharName(charName, encoding, encodingSize, code);
      if (code2 >= 0) {
	map[code] = (Gushort)code2;
	revMap[code2] = (Guchar)code;
      }
    }
  }
}

XOutputFont::~XOutputFont() {
  delete tag;
  if (xFont)
    XFreeFont(display, xFont);
  if (revMap)
    gfree(revMap);
}

//------------------------------------------------------------------------
// XOutputFontCache
//------------------------------------------------------------------------

XOutputFontCache::XOutputFontCache(Display *display1) {
  int i;

  display = display1;
  for (i = 0; i < fontCacheSize; ++i)
    fonts[i] = NULL;
  numFonts = 0;
}

XOutputFontCache::~XOutputFontCache() {
  int i;

  for (i = 0; i < numFonts; ++i)
    delete fonts[i];
}

XOutputFont *XOutputFontCache::getFont(GfxFont *gfxFont,
				       double m11, double m12,
				       double m21, double m22) {
  XOutputFont *font;
  int i, j;

  // is it the most recently used font?
  if (numFonts > 0 && fonts[0]->matches(gfxFont->getTag(),
					m11, m12, m21, m22))
    return fonts[0];

  // is it in the cache?
  for (i = 1; i < numFonts; ++i) {
    if (fonts[i]->matches(gfxFont->getTag(), m11, m12, m21, m22)) {
      font = fonts[i];
      for (j = i; j > 0; --j)
	fonts[j] = fonts[j-1];
      fonts[0] = font;
      return font;
    }
  }

  // make a new font
  font = new XOutputFont(gfxFont, m11, m12, m21, m22, display);
  if (!font->getXFont()) {
    delete font;
    return NULL;
  }

  // insert font in cache
  if (numFonts == fontCacheSize) {
    --numFonts;
    delete fonts[numFonts];
  }
  for (j = numFonts; j > 0; --j)
    fonts[j] = fonts[j-1];
  fonts[0] = font;
  ++numFonts;

  // return it
  return font;
}

//------------------------------------------------------------------------
// XOutputDev
//------------------------------------------------------------------------

XOutputDev::XOutputDev(LTKWindow *win1) {
  XGCValues gcValues;
  XColor xcolor;
  Colormap colormap;
  int r, g, b, n;
  GBool ok;

  // get pointer to X stuff
  win = win1;
  canvas = (LTKScrollingCanvas *)win->findWidget("canvas");
  display = win->getDisplay();
  screenNum = win->getScreenNum();
  pixmap = canvas->getPixmap();

  // allocate a color cube
  colormap = DefaultColormap(display, screenNum);
  if (rgbCubeSize > maxRGBCube)
    rgbCubeSize = maxRGBCube;
  ok = gFalse;
  for (numColors = rgbCubeSize; numColors >= 2; --numColors) {
    ok = gTrue;
    n = 0;
    for (r = 0; r < numColors && ok; ++r) {
      for (g = 0; g < numColors && ok; ++g) {
	for (b = 0; b < numColors && ok; ++b) {
	  xcolor.red = (r * 65535) / (numColors - 1);
	  xcolor.green = (g * 65535) / (numColors - 1);
	  xcolor.blue = (b * 65535) / (numColors - 1);
	  if (XAllocColor(display, colormap, &xcolor))
	    colors[n++] = xcolor.pixel;
	  else
	    ok = gFalse;
	}
      }
    }
    if (ok)
      break;
    XFreeColors(display, colormap, colors, n, 0);
  }
  if (!ok) {
    numColors = 1;
    colors[0] = BlackPixel(display, screenNum);
    colors[1] = WhitePixel(display, screenNum);
  }

  // allocate GCs
  gcValues.foreground = colors[0];
  gcValues.background = WhitePixel(display, screenNum);
  gcValues.line_width = 0;
  gcValues.line_style = LineSolid;
  strokeGC = XCreateGC(display, pixmap,
		       GCForeground | GCBackground | GCLineWidth | GCLineStyle,
                       &gcValues);
  fillGC = XCreateGC(display, pixmap,
		     GCForeground | GCBackground | GCLineWidth | GCLineStyle,
		     &gcValues);
  gcValues.foreground = gcValues.background;
  paperGC = XCreateGC(display, pixmap,
		      GCForeground | GCBackground | GCLineWidth | GCLineStyle,
		      &gcValues);

  // no clip region yet
  clipRegion = NULL;

  // set up the font cache and fonts
  gfxFont = NULL;
  font = NULL;
  fontCache = new XOutputFontCache(display);

  // empty state stack
  save = NULL;

  // initialize graphics state
  clear();
}

XOutputDev::~XOutputDev() {
  delete fontCache;
  XFreeGC(display, strokeGC);
  XFreeGC(display, fillGC);
  XFreeGC(display, paperGC);
  if (clipRegion)
    XDestroyRegion(clipRegion);
}

void XOutputDev::setPageSize(int x, int y) {
  canvas->resize(x, y);
  pixmap = canvas->getPixmap();
}

void XOutputDev::clear() {
  XOutputState *s;
  XGCValues gcValues;
  XRectangle rect;

  // clear state stack
  while (save) {
    s = save;
    save = save->next;
    XFreeGC(display, s->strokeGC);
    XFreeGC(display, s->fillGC);
    XDestroyRegion(s->clipRegion);
    delete s;
  }
  save = NULL;

  // reset GCs
  gcValues.foreground = colors[0];
  gcValues.background = WhitePixel(display, screenNum);
  gcValues.line_width = 0;
  gcValues.line_style = LineSolid;
  XChangeGC(display, strokeGC,
	    GCForeground | GCBackground | GCLineWidth | GCLineStyle,
	    &gcValues);
  XChangeGC(display, fillGC,
	    GCForeground | GCBackground | GCLineWidth | GCLineStyle,
	    &gcValues);

  // clear clipping region
  if (clipRegion)
    XDestroyRegion(clipRegion);
  clipRegion = XCreateRegion();
  rect.x = rect.y = 0;
  rect.width = canvas->getRealWidth();
  rect.height = canvas->getRealHeight();
  XUnionRectWithRegion(&rect, clipRegion, clipRegion);
  XSetRegion(display, strokeGC, clipRegion);
  XSetRegion(display, fillGC, clipRegion);

  // clear font
  gfxFont = NULL;
  font = NULL;

  // clear window
  XFillRectangle(display, pixmap, paperGC,
		 0, 0, canvas->getRealWidth(), canvas->getRealHeight());
}

void XOutputDev::dump() {
  canvas->redraw();
}

void XOutputDev::saveState(GfxState *state) {
  XOutputState *s;
  XGCValues values;

  // save current state
  s = new XOutputState;
  s->strokeGC = strokeGC;
  s->fillGC = fillGC;
  s->clipRegion = clipRegion;

  // push onto state stack
  s->next = save;
  save = s;

  // create a new current state by copying
  strokeGC = XCreateGC(display, pixmap, 0, &values);
  XCopyGC(display, s->strokeGC, 0xffffffff, strokeGC);
  fillGC = XCreateGC(display, pixmap, 0, &values);
  XCopyGC(display, s->fillGC, 0xffffffff, fillGC);
  clipRegion = XCreateRegion();
  XUnionRegion(s->clipRegion, clipRegion, clipRegion);
  XSetRegion(display, strokeGC, clipRegion);
  XSetRegion(display, fillGC, clipRegion);
}

void XOutputDev::restoreState(GfxState *state) {
  XOutputState *s;

  if (save) {
    // kill current state
    XFreeGC(display, strokeGC);
    XFreeGC(display, fillGC);
    XDestroyRegion(clipRegion);

    // restore state
    strokeGC = save->strokeGC;
    fillGC = save->fillGC;
    clipRegion = save->clipRegion;
    XSetRegion(display, strokeGC, clipRegion);
    XSetRegion(display, fillGC, clipRegion);

    // pop state stack
    s = save;
    save = save->next;
    delete s;
  }
}

void XOutputDev::updateAll(GfxState *state) {
  updateLineAttrs(state, gTrue);
  updateMiterLimit(state);
  updateFillColor(state);
  updateStrokeColor(state);
  updateFont(state);
}

void XOutputDev::updateCTM(GfxState *state) {
  updateLineAttrs(state, gTrue);
}

void XOutputDev::updateLineDash(GfxState *state){
  updateLineAttrs(state, gTrue);
}

void XOutputDev::updateLineJoin(GfxState *state) {
  updateLineAttrs(state, gFalse);
}

void XOutputDev::updateLineCap(GfxState *state) {
  updateLineAttrs(state, gFalse);
}

// unimplemented
void XOutputDev::updateMiterLimit(GfxState *state) {
}

void XOutputDev::updateLineWidth(GfxState *state) {
  updateLineAttrs(state, gFalse);
}

void XOutputDev::updateLineAttrs(GfxState *state, GBool updateDash) {
  double width;
  int cap, join;
  double *dashPattern;
  int dashLength;
  double dashStart;
  char dashList[20];
  int i;

  width = state->getTransformedLineWidth();
  switch (state->getLineCap()) {
  case 0: cap = CapButt; break;
  case 1: cap = CapRound; break;
  case 2: cap = CapProjecting; break;
  default:
    error(0, "Bad line cap style (%d)", state->getLineCap());
    cap = CapButt;
    break;
  }
  switch (state->getLineJoin()) {
  case 0: join = JoinMiter; break;
  case 1: join = JoinRound; break;
  case 2: join = JoinBevel; break;
  default:
    error(0, "Bad line join style (%d)", state->getLineJoin());
    join = JoinMiter;
    break;
  }
  state->getLineDash(&dashPattern, &dashLength, &dashStart);
  XSetLineAttributes(display, strokeGC, xoutRound(width),
		     dashLength > 0 ? LineOnOffDash : LineSolid,
		     cap, join);
  if (updateDash && dashLength > 0) {
    if (dashLength > 20)
      dashLength = 20;
    for (i = 0; i < dashLength; ++i) {
      dashList[i] = xoutRound(state->transformWidth(dashPattern[i]));
      if (dashList[i] == 0)
	dashList[i] = 1;
    }
    XSetDashes(display, strokeGC, xoutRound(dashStart), dashList, dashLength);
  }
}

void XOutputDev::updateFillColor(GfxState *state) {
  XSetForeground(display, fillGC, findColor(state->getFillColor()));
}

void XOutputDev::updateStrokeColor(GfxState *state) {
  XSetForeground(display, strokeGC, findColor(state->getStrokeColor()));
}

void XOutputDev::updateFont(GfxState *state) {
  double m11, m12, m21, m22;

  if (!(gfxFont = state->getFont())) {
    font = NULL;
    return;
  }
  state->getFontTransMat(&m11, &m12, &m21, &m22);
  font = fontCache->getFont(gfxFont, m11, m12, m21, m22);
  if (font) {
    XSetFont(display, fillGC, font->getXFont()->fid);
    XSetFont(display, strokeGC, font->getXFont()->fid);
  }
}

void XOutputDev::stroke(GfxState *state) {
  GfxPath *path;
  GfxSubpath *subpath;
  XPoint *points;
  double x, y;
  int n, m, i, j;

  // get path
  path = state->getPath();
  n = path->getNumSubpaths();

  // allocate points array
  m = 0;
  for (i = 0; i < n; ++i) {
    subpath = path->getSubpath(i);
    if (subpath->getNumPoints() > m)
      m = subpath->getNumPoints();
  }
  if (m <= numTmpPoints)
    points = tmpPoints;
  else
    points = (XPoint *)gmalloc(m * sizeof(XPoint));

  // draw the lines
  for (i = 0; i < n; ++i) {
    subpath = path->getSubpath(i);
    m = subpath->getNumPoints();
    for (j = 0; j < m; ++j) {
      state->transform(subpath->getX(j), subpath->getY(j), &x, &y);
      points[j].x = xoutRound(x);
      points[j].y = xoutRound(y);
    }
    XDrawLines(display, pixmap, strokeGC, points, m, CoordModeOrigin);
  }

  // free points
  if (points != tmpPoints)
    gfree(points);
}

void XOutputDev::fill(GfxState *state) {
  doFill(state, WindingRule);
}

void XOutputDev::eoFill(GfxState *state) {
  doFill(state, EvenOddRule);
}

// This contains a kludge to deal with fills with multiple subpaths.
// First, it divides up the subpaths into non-overlapping polygons by
// simply comparing bounding rectangles.  Second it handles filling
// polygons with multiple disjoint subpaths by connecting all of the
// subpaths to one point.  There really ought to be a better way of
// doing this.
void XOutputDev::doFill(GfxState *state, int rule) {
  static int tmpLengths[numTmpSubpaths];
  static BoundingRect tmpRects[numTmpSubpaths];
  GfxPath *path;
  GfxSubpath *subpath;
  XPoint *points;
  int *lengths;
  BoundingRect *rects;
  BoundingRect rect;
  double x, y;
  int x1, y1;
  int n, m, i, j, k;

  // set fill rule
  XSetFillRule(display, fillGC, rule);

  // get path
  path = state->getPath();
  n = path->getNumSubpaths();

  // allocate points and lengths arrays
  m = 0;
  for (i = 0; i < n; ++i)
    m += path->getSubpath(i)->getNumPoints() + 2;
  if (m < numTmpPoints)
    points = tmpPoints;
  else
    points = (XPoint *)gmalloc(m * sizeof(XPoint));
  if (n < numTmpSubpaths)
    lengths = tmpLengths;
  else
    lengths = (int *)gmalloc(n * sizeof(int));

  // allocate bounding rectangles array
  if (n < numTmpSubpaths)
    rects = tmpRects;
  else
    rects = (BoundingRect *)gmalloc(n * sizeof(BoundingRect));

  // transform points
  k = 0;
  for (i = 0; i < n; ++i) {

    // do one subpath
    subpath = path->getSubpath(i);
    m = subpath->getNumPoints();
    for (j = 0; j < m; ++j) {

      // transform a point
      state->transform(subpath->getX(j), subpath->getY(j), &x, &y);
      points[k+j].x = x1 = xoutRound(x);
      points[k+j].y = y1 = xoutRound(y);

      // update bounding rectangle
      if (n > 1) {
	if (j == 0) {
	  rects[i].xMin = rects[i].xMax = x1;
	  rects[i].yMin = rects[i].yMax = y1;
	} else {
	  if (x1 < rects[i].xMin)
	    rects[i].xMin = x1;
	  else if (x1 > rects[i].xMax)
	    rects[i].xMax = x1;
	  if (y1 < rects[i].yMin)
	    rects[i].yMin = y1;
	  else if (y1 > rects[i].yMax)
	    rects[i].yMax = y1;
	}
      }
    }

    // close subpath if necessary
    if (points[k].x != points[k+m-1].x || points[k].y != points[k+m-1].y) {
      points[k+m] = points[k];
      ++m;
    }
    lengths[i] = m;

    // next subpath
    k += m + 1;
  }

  // only one subpath
  if (n == 1) {
    XFillPolygon(display, pixmap, fillGC, points, lengths[0],
		 Complex, CoordModeOrigin);

  // multiple subpaths
  } else {
    i = 0;
    k = 0;
    while (i < n) {
      rect = rects[i];
      m = lengths[i];
      points[k+m] = points[k];
      ++m;
      for (j = i + 1; j < n; ++j) {
	if (!(((rects[j].xMin > rect.xMin && rects[j].xMin < rect.xMax) ||
	       (rects[j].xMax > rect.xMin && rects[j].xMax < rect.xMax) ||
	       (rects[j].xMin < rect.xMin && rects[j].xMax > rect.xMax)) &&
	      ((rects[j].yMin > rect.yMin && rects[j].yMin < rect.yMax) ||
	       (rects[j].yMax > rect.yMin && rects[j].yMax < rect.yMax) ||
	       (rects[j].yMin < rect.yMin && rects[j].yMax > rect.yMax))))
	  break;
	if (rects[j].xMin < rect.xMin)
	  rect.xMin = rects[j].xMin;
	if (rects[j].xMax > rect.xMax)
	  rect.xMax = rects[j].xMax;
	if (rects[j].yMin < rect.yMin)
	  rect.yMin = rects[j].yMin;
	if (rects[j].yMax > rect.yMax)
	  rect.yMax = rects[j].yMax;
	m += lengths[j];
	points[k+m] = points[k];
	++m;
      }
      XFillPolygon(display, pixmap, fillGC, points + k, m,
		   Complex, CoordModeOrigin);
      i = j;
      k += m;
    }
  }

  // free points, lengths, and rectangles arrays
  if (points != tmpPoints)
    gfree(points);
  if (lengths != tmpLengths)
    gfree(lengths);
  if (rects != tmpRects)
    gfree(rects);
}

void XOutputDev::clip(GfxState *state) {
  XPoint *points;
  int n;
  Region region;

  points = pathPoints(state, &n);
  region = XPolygonRegion(points, n, WindingRule);
  gfree(points);
  XIntersectRegion(region, clipRegion, clipRegion);
  XDestroyRegion(region);
  XSetRegion(display, strokeGC, clipRegion);
  XSetRegion(display, fillGC, clipRegion);
}

void XOutputDev::eoClip(GfxState *state) {
  XPoint *points;
  int n;
  Region region;

  points = pathPoints(state, &n);
  region = XPolygonRegion(points, n, EvenOddRule);
  gfree(points);
  XIntersectRegion(region, clipRegion, clipRegion);
  XDestroyRegion(region);
  XSetRegion(display, strokeGC, clipRegion);
  XSetRegion(display, fillGC, clipRegion);
}

XPoint *XOutputDev::pathPoints(GfxState *state, int *numPoints) {
  XPoint *points;
  GfxPath *path;
  GfxSubpath *subpath;
  double x, y;
  int n, m, i, j, k;

  path = state->getPath();
  n = path->getNumSubpaths();
  m = 0;
  for (i = 0; i < n; ++i)
    m += path->getSubpath(i)->getNumPoints();
  points = (XPoint *)gmalloc(m * sizeof(XPoint));
  k = 0;
  for (i = 0; i < n; ++i) {
    subpath = path->getSubpath(i);
    for (j = 0; j < subpath->getNumPoints(); ++j) {
      state->transform(subpath->getX(j), subpath->getY(j), &x, &y);
      points[k].x = xoutRound(x);
      points[k].y = xoutRound(y);
      ++k;
    }
  }
  *numPoints = m;
  return points;
}

void XOutputDev::drawChar(GfxState *state, double x, double y, Guchar c) {
  Gushort c1;
  char buf;
  char *p;
  int n, i;
  double x1, y1;
  double tx;

  if (!font)
    return;
  state->transform(x, y, &x1, &y1);
  c1 = font->mapChar(c);
  if (c1 <= lastRegularChar) {
    buf = (char)c1;
    XDrawString(display, pixmap,
		(state->getRender() & 1) ? strokeGC : fillGC,
		xoutRound(x1), xoutRound(y1), &buf, 1);
  } else if (c1 <= lastSubstChar) {
    buf = (char)substChars[c1 - firstSubstChar];
    XDrawString(display, pixmap,
		(state->getRender() & 1) ? strokeGC : fillGC,
		xoutRound(x1), xoutRound(y1), &buf, 1);
  } else if (c1 <= lastConstrChar) {
    //~ need to deal with rotated text here
    switch (c1 - firstConstrChar) {
    case 0: // bullet
      tx = 0.25 * state->getTransformedFontSize() * 
           gfxFont->getWidth(c);
      XFillRectangle(display, pixmap,
		     (state->getRender() & 1) ? strokeGC : fillGC,
		     xoutRound(x1 + tx),
		     xoutRound(y1 - 0.4 * font->getXFont()->ascent - tx),
		     xoutRound(2 * tx), xoutRound(2 * tx));
      break;
    case 1: // trademark
//~      tx = state->getTransformedFontSize() *
//~           (gfxFont->getWidth(c) -
//~            gfxFont->getWidth(font->revCharMap('M')));
      tx = 0.9 * state->getTransformedFontSize() *
           gfxFont->getWidth(font->revMapChar('T'));
      y1 -= 0.33 * (double)font->getXFont()->ascent;
      buf = 'T';
      XDrawString(display, pixmap,
		  (state->getRender() & 1) ? strokeGC : fillGC,
		  xoutRound(x1), xoutRound(y1), &buf, 1);
      x1 += tx;
      buf = 'M';
      XDrawString(display, pixmap,
		  (state->getRender() & 1) ? strokeGC : fillGC,
		  xoutRound(x1), xoutRound(y1), &buf, 1);
      break;
    }
  } else if (c1 <= lastMultiChar) {
    p = multiChars[c1 - firstMultiChar];
    n = strlen(p);
    tx = gfxFont->getWidth(c);
    for (i = 1; i < n; ++i)
      tx -= gfxFont->getWidth(font->revMapChar(p[i]));
    tx = tx * state->getTransformedFontSize() / (double)(n - 1);
    for (i = 0; i < n; ++i) {
      XDrawString(display, pixmap,
		  (state->getRender() & 1) ? strokeGC : fillGC,
		  xoutRound(x1), xoutRound(y1), p + i, 1);
      x1 += tx;
    }
  }
}

void XOutputDev::drawImageMask(GfxState *state, Stream *str,
			       int width, int height, GBool invert) {
  XImage *image;
  int x0, y0;			// top left corner of image
  int w0, h0, w1, h1;		// size of image
  Guint depth;
  double xt, yt, wt, ht;
  GBool rotate, xFlip, yFlip;
  int x, y;
  int ix, iy;
  int px1, px2, qx, dx;
  int py1, py2, qy, dy;
  Guchar *pixLine;
  Gulong color;
  Gulong buf;
  int i, j;

  // get image position and size
  state->transform(0, 0, &xt, &yt);
  state->transformDelta(1, 1, &wt, &ht);
  if (wt > 0) {
    x0 = xoutRound(xt);
    w0 = xoutRound(wt);
  } else {
    x0 = xoutRound(xt + wt);
    w0 = xoutRound(-wt);
  }
  if (ht > 0) {
    y0 = xoutRound(yt);
    h0 = xoutRound(ht);
  } else {
    y0 = xoutRound(yt + ht);
    h0 = xoutRound(-ht);
  }
  state->transformDelta(1, 0, &xt, &yt);
  rotate = fabs(xt) < fabs(yt);
  if (rotate) {
    w1 = h0;
    h1 = w0;
    xFlip = ht < 0;
    yFlip = wt > 0;
  } else {
    w1 = w0;
    h1 = h0;
    xFlip = wt < 0;
    yFlip = ht > 0;
  }

  // set up
  str->reset();
  color = findColor(state->getFillColor());

  // check for tiny (zero width or height) images
  if (w0 == 0 || h0 == 0) {
    j = height * ((width + 7) / 8);
    for (i = 0; i < j; ++i)
      str->getChar();
    return;
  }

  // Bresenham parameters
  px1 = w1 / width;
  px2 = w1 - px1 * width;
  py1 = h1 / height;
  py2 = h1 - py1 * height;

  // allocate XImage
  depth = DefaultDepth(display, screenNum);
  if (x0 < 0 || x0 + w0 > canvas->getRealWidth() ||
      y0 < 0 || y0 + h0 > canvas->getRealHeight()) {
    error(0, "Badly placed image mask");
    return;
  }
  image = XGetImage(display, pixmap, x0, y0, w0, h0,
		    (1 << depth) - 1, ZPixmap);

  // allocate line buffer
  pixLine = (Guchar *)gmalloc(((width + 7) & ~7) * sizeof(Guchar));

  // first line (column)
  y = yFlip ? h1 - 1 : 0;
  qy = 0;

  // read image
  for (i = 0; i < height; ++i) {

    // vertical (horizontal) Bresenham
    dy = py1;
    if ((qy += py2) >= height) {
      ++dy;
      qy -= height;
    }

    // first column (line)
    x = xFlip ? w1 - 1 : 0;
    qx = 0;

    // read a line (column)
    for (j = 0; j < width; j += 8) {
      buf = str->getChar();
      if (invert)
	buf = buf ^ 0xff;
      pixLine[j+7] = buf & 1;
      buf >>= 1;
      pixLine[j+6] = buf & 1;
      buf >>= 1;
      pixLine[j+5] = buf & 1;
      buf >>= 1;
      pixLine[j+4] = buf & 1;
      buf >>= 1;
      pixLine[j+3] = buf & 1;
      buf >>= 1;
      pixLine[j+2] = buf & 1;
      buf >>= 1;
      pixLine[j+1] = buf & 1;
      buf >>= 1;
      pixLine[j] = buf & 1;
    }

    // draw line (column) in XImage
    if (dy > 0) {

      // for each column (line)...
      for (j = 0; j < width; ++j) {

	// horizontal (vertical) Bresenham
	dx = px1;
	if ((qx += px2) >= width) {
	  ++dx;
	  qx -= width;
	}

	// draw image pixel
	if (dx > 0 && pixLine[j] == 0) {
	  if (dx == 1 && dy == 1) {
	    if (rotate)
	      XPutPixel(image, y, x, color);
	    else
	      XPutPixel(image, x, y, color);
	  } else {
	    for (ix = 0; ix < dx; ++ix) {
	      for (iy = 0; iy < dy; ++iy) {
		if (rotate)
		  XPutPixel(image, yFlip ? y - iy : y + iy,
			    xFlip ? x - ix : x + ix, color);
		else
		  XPutPixel(image, xFlip ? x - ix : x + ix,
			    yFlip ? y - iy : y + iy, color);
	      }
	    }
	  }
	}

	// next column (line)
	if (xFlip)
	  x -= dx;
	else
	  x += dx;
      }
    }

    // next line (column)
    if (yFlip)
      y -= dy;
    else
      y += dy;
  }

  // blit the image into the pixmap
  XPutImage(display, pixmap, fillGC, image, 0, 0, x0, y0, w0, h0);

  // free memory
  XDestroyImage(image);
  gfree(pixLine);
}

inline Gulong XOutputDev::findColor(RGBColor *x, RGBColor *err) {
  int r, g, b;
  double gray;
  Gulong pixel;

  if (numColors == 1) {
    gray = 0.299 * x->r + 0.587 * x->g + 0.114 * x->b;
    if (gray < 0.5) {
      pixel = colors[0];
      err->r = x->r;
      err->g = x->g;
      err->b = x->b;
    } else {
      pixel = colors[1];
      err->r = x->r - 255;
      err->g = x->g - 255;
      err->b = x->b - 255;
    }
  } else {
    r = (x->r * (numColors - 1) + 128) >> 8;
    g = (x->g * (numColors - 1) + 128) >> 8;
    b = (x->b * (numColors - 1) + 128) >> 8;
    pixel = colors[(r * numColors + g) * numColors + b];
    err->r = x->r - ((r << 8) - r) / (numColors - 1);
    err->g = x->g - ((g << 8) - g) / (numColors - 1); 
    err->b = x->b - ((b << 8) - b) / (numColors - 1);
  }
  return pixel;
}

void XOutputDev::drawImage(GfxState *state, Stream *str, int width,
			   int height, GfxColorSpace *colorSpace) {
  XImage *image;
  int x0, y0;			// top left corner of image
  int w0, h0, w1, h1;		// size of image
  Guint depth;
  double xt, yt, wt, ht;
  GBool rotate, xFlip, yFlip;
  GBool dither;
  int x, y;
  int ix, iy;
  int px1, px2, qx, dx;
  int py1, py2, qy, dy;
  Guchar *pixLine;
  Gulong pixel;
  Gulong buf, bitMask;
  int bits;
  int nComps, nVals, nBits;
  Guchar r1, g1, b1;
  RGBColor color2, err;
  RGBColor *errRight, *errDown;
  int i, j, k;

  // get image position and size
  state->transform(0, 0, &xt, &yt);
  state->transformDelta(1, 1, &wt, &ht);
  if (wt > 0) {
    x0 = xoutRound(xt);
    w0 = xoutRound(wt);
  } else {
    x0 = xoutRound(xt + wt);
    w0 = xoutRound(-wt);
  }
  if (ht > 0) {
    y0 = xoutRound(yt);
    h0 = xoutRound(ht);
  } else {
    y0 = xoutRound(yt + ht);
    h0 = xoutRound(-ht);
  }
  state->transformDelta(1, 0, &xt, &yt);
  rotate = fabs(xt) < fabs(yt);
  if (rotate) {
    w1 = h0;
    h1 = w0;
    xFlip = ht < 0;
    yFlip = wt > 0;
  } else {
    w1 = w0;
    h1 = h0;
    xFlip = wt < 0;
    yFlip = ht > 0;
  }

  // set up
  str->reset();
  nComps = colorSpace->getNumComponents();
  nVals = width * nComps;
  nBits = colorSpace->getBits();
  dither = nComps > 1 || nBits > 1;

  // check for tiny (zero width or height) images
  if (w0 == 0 || h0 == 0) {
    k = height * ((nVals * nBits + 7) / 8);
    for (i = 0; i < k; ++i)
      str->getChar();
    return;
  }

  // Bresenham parameters
  px1 = w1 / width;
  px2 = w1 - px1 * width;
  py1 = h1 / height;
  py2 = h1 - py1 * height;

  // allocate XImage
  depth = DefaultDepth(display, screenNum);
  image = XCreateImage(display, DefaultVisual(display, screenNum),
		       depth, ZPixmap, 0, NULL, w0, h0, 8, 0);
  image->data = (char *)gmalloc(h0 * image->bytes_per_line);

  // allocate line buffer
  pixLine = (Guchar *)gmalloc(((nVals + 7) & ~7) * sizeof(Guchar));

  // allocate error diffusion accumulators
  if (dither) {
    errDown = (RGBColor *)gmalloc(w1 * sizeof(RGBColor));
    errRight = (RGBColor *)gmalloc((py1 + 1) * sizeof(RGBColor));
    for (j = 0; j < w1; ++j)
      errDown[j].r = errDown[j].g = errDown[j].b = 0;
  } else {
    errDown = NULL;
    errRight = NULL;
  }

  // first line (column)
  y = yFlip ? h1 - 1 : 0;
  qy = 0;

  // read image
  for (i = 0; i < height; ++i) {

    // vertical (horizontal) Bresenham
    dy = py1;
    if ((qy += py2) >= height) {
      ++dy;
      qy -= height;
    }

    // first column (line)
    x = xFlip ? w1 - 1 : 0;
    qx = 0;

    // read a line (column)
    if (nBits == 1) {
      for (j = 0; j < nVals; j += 8) {
	buf = str->getChar();
	pixLine[j+7] = buf & 1;
	buf >>= 1;
	pixLine[j+6] = buf & 1;
	buf >>= 1;
	pixLine[j+5] = buf & 1;
	buf >>= 1;
	pixLine[j+4] = buf & 1;
	buf >>= 1;
	pixLine[j+3] = buf & 1;
	buf >>= 1;
	pixLine[j+2] = buf & 1;
	buf >>= 1;
	pixLine[j+1] = buf & 1;
	buf >>= 1;
	pixLine[j] = buf & 1;
      }
    } else if (nBits == 8) {
      for (j = 0; j < nVals; ++j)
	pixLine[j] = str->getChar();
    } else {
      bitMask = (1 << nBits) - 1;
      buf = 0;
      bits = 0;
      for (j = 0; j < nVals; ++j) {
	if (bits < nBits) {
	  buf = (buf << 8) | (str->getChar() & 0xff);
	  bits += 8;
	}
	pixLine[j] = (buf >> (bits - nBits)) & bitMask;
	bits -= nBits;
      }
    }

    // draw line (column) in XImage
    if (dy > 0) {

      // clear error accumulator
      if (dither) {
	for (j = 0; j <= py1; ++j)
	  errRight[j].r = errRight[j].g = errRight[j].b = 0;
      }

      // for each column (line)...
      for (j = 0, k = 0; j < width; ++j, k += nComps) {

	// horizontal (vertical) Bresenham
	dx = px1;
	if ((qx += px2) >= width) {
	  ++dx;
	  qx -= width;
	}

	// draw image pixel
	if (dx > 0) {
	  colorSpace->getRGB(&pixLine[k], &r1, &g1, &b1);
	  if (dither) {
	    pixel = 0;
	  } else {
	    color2.r = r1;
	    color2.g = g1;
	    color2.b = b1;
	    pixel = findColor(&color2, &err);
	  }
	  if (dx == 1 && dy == 1) {
	    if (dither) {
	      color2.r = r1 + errRight[0].r + errDown[x].r;
	      if (color2.r > 255)
		color2.r = 255;
	      color2.g = g1 + errRight[0].g + errDown[x].g;
	      if (color2.g > 255)
		color2.g = 255;
	      color2.b = b1 + errRight[0].b + errDown[x].b;
	      if (color2.b > 255)
		color2.b = 255;
	      pixel = findColor(&color2, &err);
	      errRight[0].r = err.r / 2;
	      errRight[0].g = err.g / 2;
	      errRight[0].b = err.b / 2;
	      errDown[x].r = err.r - errRight[0].r;
	      errDown[x].g = err.g - errRight[0].g;
	      errDown[x].b = err.b - errRight[0].b;
	    }
	    if (rotate)
	      XPutPixel(image, y, x, pixel);
	    else
	      XPutPixel(image, x, y, pixel);
	  } else {
	    for (iy = 0; iy < dy; ++iy) {
	      for (ix = 0; ix < dx; ++ix) {
		if (dither) {
		  color2.r = r1 + errRight[iy].r +
		    errDown[xFlip ? x - ix : x + ix].r;
		  if (color2.r > 255)
		    color2.r = 255;
		  color2.g = g1 + errRight[iy].g +
		    errDown[xFlip ? x - ix : x + ix].g;
		  if (color2.g > 255)
		    color2.g = 255;
		  color2.b = b1 + errRight[iy].b +
		    errDown[xFlip ? x - ix : x + ix].b;
		  if (color2.b > 255)
		    color2.b = 255;
		  pixel = findColor(&color2, &err);
		  errRight[iy].r = err.r / 2;
		  errRight[iy].g = err.g / 2;
		  errRight[iy].b = err.b / 2;
		  errDown[xFlip ? x - ix : x + ix].r = err.r - errRight[iy].r;
		  errDown[xFlip ? x - ix : x + ix].g = err.g - errRight[iy].g;
		  errDown[xFlip ? x - ix : x + ix].b = err.b - errRight[iy].b;
		}
		if (rotate)
		  XPutPixel(image, yFlip ? y - iy : y + iy,
			    xFlip ? x - ix : x + ix, pixel);
		else
		  XPutPixel(image, xFlip ? x - ix : x + ix,
			    yFlip ? y - iy : y + iy, pixel);
	      }
	    }
	  }
	}

	// next column (line)
	if (xFlip)
	  x -= dx;
	else
	  x += dx;
      }
    }

    // next line (column)
    if (yFlip)
      y -= dy;
    else
      y += dy;
  }

  // blit the image into the pixmap
  XPutImage(display, pixmap, fillGC, image, 0, 0, x0, y0, w0, h0);

  // free memory
  gfree(image->data);
  image->data = NULL;
  XDestroyImage(image);
  gfree(pixLine);
  gfree(errRight);
  gfree(errDown);
}

Gulong XOutputDev::findColor(GfxColor *color) {
  int r, g, b;
  double gray;
  Gulong pixel;

  if (numColors == 1) {
    gray = color->getGray();
    if (gray < 0.5)
      pixel = colors[0];
    else
      pixel = colors[1];
  } else {
    r = xoutRound(color->getR() * (numColors - 1));
    g = xoutRound(color->getG() * (numColors - 1));
    b = xoutRound(color->getB() * (numColors - 1));
    pixel = colors[(r * numColors + g) * numColors + b];
  }
  return pixel;
}
