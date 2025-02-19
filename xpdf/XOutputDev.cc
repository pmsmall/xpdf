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
#include "Params.h"
#include "TextOutputDev.h"
#include "XOutputDev.h"

#include "XOutputFontInfo.h"

//------------------------------------------------------------------------
// Constants and macros
//------------------------------------------------------------------------

#define xoutRound(x) ((int)(x + 0.5))

#define maxCurveSplits 6	// max number of splits when recursively
				//   drawing Bezier curves

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
  GfxFontEncoding *encoding;
};

static FontMapEntry fontMap[] = {
  {"Courier",               "-*-courier-medium-r-*-*-%s-*-*-*-*-*-iso8859-1",
   &isoLatin1Encoding},
  {"Courier-Bold",          "-*-courier-bold-r-*-*-%s-*-*-*-*-*-iso8859-1",
   &isoLatin1Encoding},
  {"Courier-BoldOblique",   "-*-courier-bold-o-*-*-%s-*-*-*-*-*-iso8859-1",
   &isoLatin1Encoding},
  {"Courier-Oblique",       "-*-courier-medium-o-*-*-%s-*-*-*-*-*-iso8859-1",
   &isoLatin1Encoding},
  {"Helvetica",             "-*-helvetica-medium-r-*-*-%s-*-*-*-*-*-iso8859-1",
   &isoLatin1Encoding},
  {"Helvetica-Bold",        "-*-helvetica-bold-r-*-*-%s-*-*-*-*-*-iso8859-1",
   &isoLatin1Encoding},
  {"Helvetica-BoldOblique", "-*-helvetica-bold-o-*-*-%s-*-*-*-*-*-iso8859-1",
   &isoLatin1Encoding},
  {"Helvetica-Oblique",     "-*-helvetica-medium-o-*-*-%s-*-*-*-*-*-iso8859-1",
   &isoLatin1Encoding},
  {"Symbol",                "-*-symbol-medium-r-*-*-%s-*-*-*-*-*-adobe-fontspecific",
   &symbolEncoding},
  {"Times-Bold",            "-*-times-bold-r-*-*-%s-*-*-*-*-*-iso8859-1",
   &isoLatin1Encoding},
  {"Times-BoldItalic",      "-*-times-bold-i-*-*-%s-*-*-*-*-*-iso8859-1",
   &isoLatin1Encoding},
  {"Times-Italic",          "-*-times-medium-i-*-*-%s-*-*-*-*-*-iso8859-1",
   &isoLatin1Encoding},
  {"Times-Roman",           "-*-times-medium-r-*-*-%s-*-*-*-*-*-iso8859-1",
   &isoLatin1Encoding},
  {"ZapfDingbats",          "-*-zapfdingbats-medium-r-*-*-%s-*-*-*-*-*-*-*",
   &zapfDingbatsEncoding},
  {NULL}
};

static FontMapEntry *userFontMap;

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
  0xad,				// 102: hyphen --> endash
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
  GfxFontEncoding *encoding;
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

  // construct X font name
  pdfFont = gfxFont->getName();
  if (pdfFont) {
    for (p = fontMap; p->pdfFont; ++p) {
      if (!pdfFont->cmp(p->pdfFont))
	break;
    }
    if (!p->pdfFont) {
      for (p = userFontMap; p->pdfFont; ++p) {
	if (!pdfFont->cmp(p->pdfFont))
	  break;
      }
    }
  } else {
    p = NULL;
  }
  if (p && p->pdfFont) {
    fontNameFmt = p->xFont;
    encoding = p->encoding;
  } else {
    encoding = &isoLatin1Encoding;
//~ Some non-symbolic fonts are tagged as symbolic.
//    if (gfxFont->isSymbolic()) {
//      index = 12;
//      encoding = symbolEncoding;
//    } else
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
    for (sz = startSize; sz >= startSize/2 && sz >= 1; --sz) {
      sprintf(fontSize, "%d", sz);
      sprintf(fontName, fontNameFmt, fontSize);
      if ((xFont = XLoadQueryFont(display, fontName)))
	break;
    }
    if (!xFont) {
      for (sz = startSize + 1; sz < startSize + 10; ++sz) {
	sprintf(fontSize, "%d", sz);
	sprintf(fontName, fontNameFmt, fontSize);
	if ((xFont = XLoadQueryFont(display, fontName)))
	  break;
      }
      if (!xFont) {
	sprintf(fontSize, "%d", startSize);
	sprintf(fontName, fontNameFmt, fontSize);
	error(-1, "Failed to open font: '%s'", fontName);
	return;
      }
    }
  }

  // construct forward and reverse map
  for (code = 0; code < 256; ++code)
    revMap[code] = 0;
  if (encoding) {
    code2 = 0; // to make gcc happy
    for (code = 0; code < 256; ++code) {
      if ((charName = gfxFont->getCharName(code)) &&
	  (code2 = encoding->getCharCode(charName)) >= 0) {
	map[code] = (Gushort)code2;
	if (code2 < 256)
	  revMap[code2] = (Guchar)code;
      } else {
	map[code] = 0;
//~ for font debugging
//	error(-1, "font '%s', no char '%s'",
//	      pdfFont->getCString(), charName);
      }
    }
  } else {
    code2 = 0; // to make gcc happy
    //~ this is a hack to get around the fact that X won't draw
    //~ chars 0..31; this works when the fonts have duplicate encodings
    //~ for those chars
    for (code = 0; code < 32; ++code) {
      if ((charName = gfxFont->getCharName(code)) &&
	  (code2 = gfxFont->getCharCode(charName)) >= 0) {
	map[code] = (Gushort)code2;
	if (code2 < 256)
	  revMap[code2] = (Guchar)code;
      }
    }
    for (code = 32; code < 256; ++code) {
      map[code] = (Gushort)code;
      revMap[code] = (Guchar)code;
    }
  }
}

XOutputFont::~XOutputFont() {
  delete tag;
  if (xFont)
    XFreeFont(display, xFont);
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
  int r, g, b, n, m, i;
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

  // get user font map
  for (n = 0; devFontMap[n].pdfFont; ++n) ;
  userFontMap = (FontMapEntry *)gmalloc((n+1) * sizeof(FontMapEntry));
  for (i = 0; i < n; ++i) {
    userFontMap[i].pdfFont = devFontMap[i].pdfFont;
    userFontMap[i].xFont = devFontMap[i].devFont;
    m = strlen(userFontMap[i].xFont);
    if (m >= 10 && !strcmp(userFontMap[i].xFont + m - 10, "-iso8859-2"))
      userFontMap[i].encoding = &isoLatin2Encoding;
    else if (m >= 13 && !strcmp(userFontMap[i].xFont + m - 13,
				"-fontspecific"))
      userFontMap[i].encoding = NULL;
    else
      userFontMap[i].encoding = &isoLatin1Encoding;
  }
  userFontMap[n].pdfFont = NULL;

  // set up the font cache and fonts
  gfxFont = NULL;
  font = NULL;
  fontCache = new XOutputFontCache(display);

  // empty state stack
  save = NULL;

  // create text object
  text = new TextPage(gFalse);
}

XOutputDev::~XOutputDev() {
  gfree(userFontMap);
  delete fontCache;
  XFreeGC(display, strokeGC);
  XFreeGC(display, fillGC);
  XFreeGC(display, paperGC);
  if (clipRegion)
    XDestroyRegion(clipRegion);
  delete text;
}

void XOutputDev::startPage(int pageNum, GfxState *state) {
  XOutputState *s;
  XGCValues gcValues;
  XRectangle rect;

  // set page size
  canvas->resize(state->getPageWidth(), state->getPageHeight());
  pixmap = canvas->getPixmap();

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

  // default line flatness
  flatness = 0;

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
  canvas->redraw();

  // clear text object
  text->clear();
}

void XOutputDev::endPage() {
  text->coalesce();
}

void XOutputDev::dump() {
  canvas->redraw();
}

void XOutputDev::drawLinkBorder(double x1, double y1, double x2, double y2,
				double w) {
  GfxColor color;
  XPoint points[5];
  int x, y;

  color.setRGB(0, 0, 1);
  XSetForeground(display, strokeGC, findColor(&color));
  XSetLineAttributes(display, strokeGC, xoutRound(w),
		     LineSolid, CapRound, JoinRound);
  cvtUserToDev(x1, y1, &x, &y);
  points[0].x = points[4].x = x;
  points[0].y = points[4].y = y;
  cvtUserToDev(x2, y1, &x, &y);
  points[1].x = x;
  points[1].y = y;
  cvtUserToDev(x2, y2, &x, &y);
  points[2].x = x;
  points[2].y = y;
  cvtUserToDev(x1, y2, &x, &y);
  points[3].x = x;
  points[3].y = y;
  XDrawLines(display, pixmap, strokeGC, points, 5, CoordModeOrigin);
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
    flatness = state->getFlatness();
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
  updateFlatness(state);
  updateMiterLimit(state);
  updateFillColor(state);
  updateStrokeColor(state);
  updateFont(state);
}

void XOutputDev::updateCTM(GfxState *state, double m11, double m12,
			   double m21, double m22, double m31, double m32) {
  updateLineAttrs(state, gTrue);
}

void XOutputDev::updateLineDash(GfxState *state) {
  updateLineAttrs(state, gTrue);
}

void XOutputDev::updateFlatness(GfxState *state) {
  flatness = state->getFlatness();
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
    error(-1, "Bad line cap style (%d)", state->getLineCap());
    cap = CapButt;
    break;
  }
  switch (state->getLineJoin()) {
  case 0: join = JoinMiter; break;
  case 1: join = JoinRound; break;
  case 2: join = JoinBevel; break;
  default:
    error(-1, "Bad line join style (%d)", state->getLineJoin());
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
  m11 *= state->getHorizScaling();
  m21 *= state->getHorizScaling();
  font = fontCache->getFont(gfxFont, m11, m12, m21, m22);
  if (font) {
    XSetFont(display, fillGC, font->getXFont()->fid);
    XSetFont(display, strokeGC, font->getXFont()->fid);
  }
}

void XOutputDev::stroke(GfxState *state) {
  XPoint *points;
  int *lengths;
  int n, size, numPoints, i, j;

  // transform points
  n = convertPath(state, &points, &size, &numPoints, &lengths, gFalse);

  // draw each subpath
  j = 0;
  for (i = 0; i < n; ++i) {
    XDrawLines(display, pixmap, strokeGC, points + j, lengths[i],
	       CoordModeOrigin);
    j += lengths[i];
  }

  // free points and lengths arrays
  if (points != tmpPoints)
    gfree(points);
  if (lengths != tmpLengths)
    gfree(lengths);
}

void XOutputDev::fill(GfxState *state) {
  doFill(state, WindingRule);
}

void XOutputDev::eoFill(GfxState *state) {
  doFill(state, EvenOddRule);
}

//
//  X doesn't color the pixels on the right-most and bottom-most
//  borders of a polygon.  This means that one-pixel-thick polygons
//  are not colored at all.  I think this is supposed to be a
//  feature, but I can't figure out why.  So after it fills a
//  polygon, it also draws lines around the border.  This is done
//  only for single-component polygons, since it's not very
//  compatible with the compound polygon kludge (see convertPath()).
//
void XOutputDev::doFill(GfxState *state, int rule) {
  XPoint *points;
  int *lengths;
  int n, size, numPoints, i, j;

  // set fill rule
  XSetFillRule(display, fillGC, rule);

  // transform points, build separate polygons
  n = convertPath(state, &points, &size, &numPoints, &lengths, gTrue);

  // fill them
  j = 0;
  for (i = 0; i < n; ++i) {
    XFillPolygon(display, pixmap, fillGC, points + j, lengths[i],
		 Complex, CoordModeOrigin);
    if (state->getPath()->getNumSubpaths() == 1) {
      XDrawLines(display, pixmap, fillGC, points + j, lengths[i],
		 CoordModeOrigin);
    }
    j += lengths[i] + 1;
  }

  // free points and lengths arrays
  if (points != tmpPoints)
    gfree(points);
  if (lengths != tmpLengths)
    gfree(lengths);
}

void XOutputDev::clip(GfxState *state) {
  doClip(state, WindingRule);
}

void XOutputDev::eoClip(GfxState *state) {
  doClip(state, EvenOddRule);
}

void XOutputDev::doClip(GfxState *state, int rule) {
  Region region, region2;
  XPoint *points;
  int *lengths;
  int n, size, numPoints, i, j;

  // transform points, build separate polygons
  n = convertPath(state, &points, &size, &numPoints, &lengths, gTrue);

  // construct union of subpath regions
  region = XPolygonRegion(points, lengths[0], rule);
  j = lengths[0] + 1;
  for (i = 1; i < n; ++i) {
    region2 = XPolygonRegion(points + j, lengths[i], rule);
    XUnionRegion(region2, region, region);
    XDestroyRegion(region2);
    j += lengths[i] + 1;
  }

  // intersect region with clipping region
  XIntersectRegion(region, clipRegion, clipRegion);
  XDestroyRegion(region);
  XSetRegion(display, strokeGC, clipRegion);
  XSetRegion(display, fillGC, clipRegion);

  // free points and lengths arrays
  if (points != tmpPoints)
    gfree(points);
  if (lengths != tmpLengths)
    gfree(lengths);
}

//
// Transform points in the path and convert curves to line segments.
// Builds a set of subpaths and returns the number of subpaths.
// If <fill> is set, close any unclosed subpaths and activate a kludge
// for polygon fills:  First, it divides up the subpaths into
// non-overlapping polygons by simply comparing bounding rectangles.
// Then it connects subaths within a single compound polygon to a single
// point so that X can fill the polygon (sort of).
//
int XOutputDev::convertPath(GfxState *state, XPoint **points, int *size,
			    int *numPoints, int **lengths, GBool fill) {
  GfxPath *path;
  BoundingRect *rects;
  BoundingRect rect;
  int n, i, ii, j, k, k0;

  // get path and number of subpaths
  path = state->getPath();
  n = path->getNumSubpaths();

  // allocate lengths array
  if (n < numTmpSubpaths)
    *lengths = tmpLengths;
  else
    *lengths = (int *)gmalloc(n * sizeof(int));

  // allocate bounding rectangles array
  if (fill) {
    if (n < numTmpSubpaths)
      rects = tmpRects;
    else
      rects = (BoundingRect *)gmalloc(n * sizeof(BoundingRect));
  } else {
    rects = NULL;
  }

  // do each subpath
  *points = tmpPoints;
  *size = numTmpPoints;
  *numPoints = 0;
  for (i = 0; i < n; ++i) {

    // transform the points
    j = *numPoints;
    convertSubpath(state, path->getSubpath(i), points, size, numPoints);

    // construct bounding rectangle
    if (fill) {
      rects[i].xMin = rects[i].xMax = (*points)[j].x;
      rects[i].yMin = rects[i].yMax = (*points)[j].y;
      for (k = j + 1; k < *numPoints; ++k) {
	if ((*points)[k].x < rects[i].xMin)
	  rects[i].xMin = (*points)[k].x;
	else if ((*points)[k].x > rects[i].xMax)
	  rects[i].xMax = (*points)[k].x;
	if ((*points)[k].y < rects[i].yMin)
	  rects[i].yMin = (*points)[k].y;
	else if ((*points)[k].y > rects[i].yMax)
	  rects[i].yMax = (*points)[k].y;
      }
    }

    // close subpath if necessary
    if (fill && ((*points)[*numPoints-1].x != (*points)[j].x ||
		 (*points)[*numPoints-1].y != (*points)[j].y)) {
      addPoint(points, size, numPoints, (*points)[j].x, (*points)[j].y);
    }

    // length of this subpath
    (*lengths)[i] = *numPoints - j;

    // leave an extra point for compound fill hack
    if (fill)
      addPoint(points, size, numPoints, 0, 0);
  }

  // combine compound polygons
  if (fill) {
    i = j = k = 0;
    while (i < n) {

      // start with subpath i
      rect = rects[i];
      (*lengths)[j] = (*lengths)[i];
      k0 = k;
      (*points)[k + (*lengths)[i]] = (*points)[k0];
      k += (*lengths)[i] + 1;
      ++i;

      // combine overlapping polygons
      do {

	// look for the first subsequent subpath, if any, which overlaps
	for (ii = i; ii < n; ++ii) {
	  if (((rects[ii].xMin > rect.xMin && rects[ii].xMin < rect.xMax) ||
	       (rects[ii].xMax > rect.xMin && rects[ii].xMax < rect.xMax) ||
	       (rects[ii].xMin < rect.xMin && rects[ii].xMax > rect.xMax)) &&
	      ((rects[ii].yMin > rect.yMin && rects[ii].yMin < rect.yMax) ||
	       (rects[ii].yMax > rect.yMin && rects[ii].yMax < rect.yMax) ||
	       (rects[ii].yMin < rect.yMin && rects[ii].yMax > rect.yMax)))
	    break;
	}

	// if there is an overlap, combine the polygons
	if (ii < n) {
	  for (; i <= ii; ++i) {
	    if (rects[i].xMin < rect.xMin)
	      rect.xMin = rects[j].xMin;
	    if (rects[i].xMax > rect.xMax)
	      rect.xMax = rects[j].xMax;
	    if (rects[i].yMin < rect.yMin)
	      rect.yMin = rects[j].yMin;
	    if (rects[i].yMax > rect.yMax)
	      rect.yMax = rects[j].yMax;
	    (*lengths)[j] += (*lengths)[i] + 1;
	    (*points)[k + (*lengths)[i]] = (*points)[k0];
	    k += (*lengths)[i] + 1;
	  }
	}
      } while (ii < n && i < n);

      ++j;
    }

    // free bounding rectangles
    if (rects != tmpRects)
      gfree(rects);

    n = j;
  }

  return n;
}

//
// Transform points in a single subpath and convert curves to line
// segments.
//
void XOutputDev::convertSubpath(GfxState *state, GfxSubpath *subpath,
				XPoint **points, int *size, int *n) {
  double x0, y0, x1, y1, x2, y2, x3, y3;
  int m, i;

  m = subpath->getNumPoints();
  i = 0;
  while (i < m) {
    if (i >= 1 && subpath->getCurve(i)) {
      state->transform(subpath->getX(i-1), subpath->getY(i-1), &x0, &y0);
      state->transform(subpath->getX(i), subpath->getY(i), &x1, &y1);
      state->transform(subpath->getX(i+1), subpath->getY(i+1), &x2, &y2);
      state->transform(subpath->getX(i+2), subpath->getY(i+2), &x3, &y3);
      doCurve(points, size, n, x0, y0, x1, y1, x2, y2, x3, y3);
      i += 3;
    } else {
      state->transform(subpath->getX(i), subpath->getY(i), &x1, &y1);
      addPoint(points, size, n, xoutRound(x1), xoutRound(y1));
      ++i;
    }
  }
}

//
// Subdivide a Bezier curve.  This uses floating point to avoid
// propagating rounding errors.  (The curves look noticeably more
// jagged with integer arithmetic.)
//
void XOutputDev::doCurve(XPoint **points, int *size, int *n,
			 double x0, double y0, double x1, double y1,
			 double x2, double y2, double x3, double y3) {
  double x[(1<<maxCurveSplits)+1][3];
  double y[(1<<maxCurveSplits)+1][3];
  int next[1<<maxCurveSplits];
  int p1, p2, p3;
  double xx1, yy1, xx2, yy2;
  double dx, dy, mx, my, d1, d2;
  double xl0, yl0, xl1, yl1, xl2, yl2;
  double xr0, yr0, xr1, yr1, xr2, yr2, xr3, yr3;
  double xh, yh;
  double flat;

  flat = (double)(flatness * flatness);

  // initial segment
  p1 = 0;
  p2 = 1<<maxCurveSplits;
  x[p1][0] = x0;  y[p1][0] = y0;
  x[p1][1] = x1;  y[p1][1] = y1;
  x[p1][2] = x2;  y[p1][2] = y2;
  x[p2][0] = x3;  y[p2][0] = y3;
  next[p1] = p2;

  while (p1 < (1<<maxCurveSplits)) {

    // get next segment
    xl0 = x[p1][0];  yl0 = y[p1][0];
    xx1 = x[p1][1];  yy1 = y[p1][1];
    xx2 = x[p1][2];  yy2 = y[p1][2];
    p2 = next[p1];
    xr3 = x[p2][0];  yr3 = y[p2][0];

    // compute distances from control points to midpoint of the
    // straight line (this is a bit of a hack, but it's much faster
    // than computing the actual distances to the line)
    mx = (xl0 + xr3) * 0.5;
    my = (yl0 + yr3) * 0.5;
    dx = xx1 - mx;
    dy = yy1 - my;
    d1 = dx*dx + dy*dy;
    dx = xx2 - mx;
    dy = yy2 - my;
    d2 = dx*dx + dy*dy;

    // if curve is flat enough, or no more divisions allowed then
    // add the straight line segment
    if (p2 - p1 <= 1 || (d1 <= flat && d2 <= flat)) {
      addPoint(points, size, n, xoutRound(xr3), xoutRound(yr3));
      p1 = p2;

    // otherwise, subdivide the curve
    } else {
      xl1 = (xl0 + xx1) * 0.5;
      yl1 = (yl0 + yy1) * 0.5;
      xh = (xx1 + xx2) * 0.5;
      yh = (yy1 + yy2) * 0.5;
      xl2 = (xl1 + xh) * 0.5;
      yl2 = (yl1 + yh) * 0.5;
      xr2 = (xx2 + xr3) * 0.5;
      yr2 = (yy2 + yr3) * 0.5;
      xr1 = (xh + xr2) * 0.5;
      yr1 = (yh + yr2) * 0.5;
      xr0 = (xl2 + xr1) * 0.5;
      yr0 = (yl2 + yr1) * 0.5;

      // add the new subdivision points
      p3 = (p1 + p2) / 2;
      x[p1][1] = xl1;  y[p1][1] = yl1;
      x[p1][2] = xl2;  y[p1][2] = yl2;
      next[p1] = p3;
      x[p3][0] = xr0;  y[p3][0] = yr0;
      x[p3][1] = xr1;  y[p3][1] = yr1;
      x[p3][2] = xr2;  y[p3][2] = yr2;
      next[p3] = p2;
    }
  }
}

//
// Add a point to the points array.  (This would use a generic resizable
// array type if C++ supported parameterized types in some reasonable
// way -- templates are a disgusting kludge.)
//
void XOutputDev::addPoint(XPoint **points, int *size, int *k, int x, int y) {
  if (*k >= *size) {
    *size += 32;
    if (*points == tmpPoints) {
      *points = (XPoint *)gmalloc(*size * sizeof(XPoint));
      memcpy(*points, tmpPoints, *k * sizeof(XPoint));
    } else {
      *points = (XPoint *)grealloc(*points, *size * sizeof(XPoint));
    }
  }
  (*points)[*k].x = x;
  (*points)[*k].y = y;
  ++(*k);
}

void XOutputDev::beginString(GfxState *state, GString *s) {
  text->beginString(state, s);
}

void XOutputDev::endString(GfxState *state) {
  text->endString();
}

void XOutputDev::drawChar(GfxState *state, double x, double y,
			  double dx, double dy, Guchar c) {
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
//~ this should use a smaller font
//      tx = state->getTransformedFontSize() *
//           (gfxFont->getWidth(c) -
//            gfxFont->getWidth(font->revCharMap('M')));
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
  text->addChar(state, x, y, dx, dy, c);
}

void XOutputDev::drawImageMask(GfxState *state, Stream *str,
			       int width, int height, GBool invert,
			       GBool inlineImg) {
  XImage *image;
  int x0, y0;			// top left corner of image
  int w0, h0, w1, h1;		// size of image
  int x2, y2;
  int w2, h2;
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
  image = XCreateImage(display, DefaultVisual(display, screenNum),
		       depth, ZPixmap, 0, NULL, w0, h0, 8, 0);
  image->data = (char *)gmalloc(h0 * image->bytes_per_line);
  if (x0 + w0 > canvas->getRealWidth())
    w2 = canvas->getRealWidth() - x0;
  else
    w2 = w0;
  if (x0 < 0) {
    x2 = -x0;
    w2 += x0;
    x0 = 0;
  } else {
    x2 = 0;
  }
  if (y0 + h0 > canvas->getRealHeight())
    h2 = canvas->getRealHeight() - y0;
  else
    h2 = h0;
  if (y0 < 0) {
    y2 = -y0;
    h2 += y0;
    y0 = 0;
  } else {
    y2 = 0;
  }
  XGetSubImage(display, pixmap, x0, y0, w2, h2, (1 << depth) - 1, ZPixmap,
	       image, x2, y2);

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
  XPutImage(display, pixmap, fillGC, image, x2, y2, x0, y0, w2, h2);

  // free memory
  gfree(image->data);
  image->data = NULL;
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
			   int height, GfxColorSpace *colorSpace,
			   GBool inlineImg) {
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
    // even a very light color shouldn't map to white
    if (r == numColors - 1 && g == numColors - 1 && b == numColors - 1) {
      if (color->getR() < 0.95)
	--r;
      if (color->getG() < 0.95)
	--g;
      if (color->getB() < 0.95)
	--b;
    }
    pixel = colors[(r * numColors + g) * numColors + b];
  }
  return pixel;
}

GBool XOutputDev::findText(char *s, GBool top, GBool bottom,
			   int *xMin, int *yMin, int *xMax, int *yMax) {
  double xMin1, yMin1, xMax1, yMax1;
  
  xMin1 = (double)*xMin;
  yMin1 = (double)*yMin;
  xMax1 = (double)*xMax;
  yMax1 = (double)*yMax;
  if (text->findText(s, top, bottom, &xMin1, &yMin1, &xMax1, &yMax1)) {
    *xMin = xoutRound(xMin1);
    *xMax = xoutRound(xMax1);
    *yMin = xoutRound(yMin1);
    *yMax = xoutRound(yMax1);
    return gTrue;
  }
  return gFalse;
}

GString *XOutputDev::getText(int xMin, int yMin, int xMax, int yMax) {
  return text->getText((double)xMin, (double)yMin,
		       (double)xMax, (double)yMax);
}
