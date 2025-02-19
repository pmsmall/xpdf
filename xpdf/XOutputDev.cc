//========================================================================
//
// XOutputDev.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#pragma implementation

#include <stdio.h>
#include <math.h>
#include <LTKWindow.h>
#include <LTKScrollingCanvas.h>
#include <mem.h>
#include "Object.h"
#include "Stream.h"
#include "GfxState.h"
#include "GfxFont.h"
#include "Error.h"
#include "XOutputDev.h"

#define numTmpSubpaths 16	// number of elements in temporary arrays

//------------------------------------------------------------------------
// BoundingRect
//------------------------------------------------------------------------

struct BoundingRect {
  short xMin, xMax;		// min/max x values
  short yMin, yMax;		// min/max y values
};

//------------------------------------------------------------------------
// Font map
//------------------------------------------------------------------------

struct FontMapEntry {
  char *pdfFont;
  char *xFont;
};

static FontMapEntry fontMap[] = {
  {"Courier",               "-*-courier-medium-r-*-*-%d-*-*-*-*-*-*-*"},
  {"Courier-Bold",          "-*-courier-bold-r-*-*-%d-*-*-*-*-*-*-*"},
  {"Courier-Oblique",       "-*-courier-medium-o-*-*-%d-*-*-*-*-*-*-*"},
  {"Courier-BoldOblique",   "-*-courier-bold-o-*-*-%d-*-*-*-*-*-*-*"},
  {"Helvetica",             "-*-helvetica-medium-r-*-*-%d-*-*-*-*-*-*-*"},
  {"Helvetica-Bold",        "-*-helvetica-bold-r-*-*-%d-*-*-*-*-*-*-*"},
  {"Helvetica-Oblique",     "-*-helvetica-medium-o-*-*-%d-*-*-*-*-*-*-*"},
  {"Helvetica-BoldOblique", "-*-helvetica-bold-o-*-*-%d-*-*-*-*-*-*-*"},
  {"Symbol",                "-*-symbol-medium-r-*-*-%d-*-*-*-*-*-*-*"},
  {"Times-Roman",           "-*-times-medium-r-*-*-%d-*-*-*-*-*-*-*"},
  {"Times-Bold",            "-*-times-bold-r-*-*-%d-*-*-*-*-*-*-*"},
  {"Times-Italic",          "-*-times-medium-i-*-*-%d-*-*-*-*-*-*-*"},
  {"Times-BoldItalic",      "-*-times-bold-i-*-*-%d-*-*-*-*-*-*-*"},
  {"ZapfDingbats",          "-*-zapfdingbats-*-*-*-*-%d-*-*-*-*-*-*-*"},
  {NULL}
};

//------------------------------------------------------------------------
// Font substitutions
//------------------------------------------------------------------------

struct FontSubst {
  char *xFont;
  double mWidth;
};

// index: fixed*8 + serif*4 + bold*2 + italic
static FontSubst fontSubst[16] = {
  {"-*-helvetica-medium-r-*-*-%d-*-*-*-*-*-*-*", 0.833},
  {"-*-helvetica-medium-o-*-*-%d-*-*-*-*-*-*-*", 0.833},
  {"-*-helvetica-bold-r-*-*-%d-*-*-*-*-*-*-*",   0.889},
  {"-*-helvetica-bold-o-*-*-%d-*-*-*-*-*-*-*",   0.889},
  {"-*-times-medium-r-*-*-%d-*-*-*-*-*-*-*",     0.788},
  {"-*-times-medium-i-*-*-%d-*-*-*-*-*-*-*",     0.722},
  {"-*-times-bold-r-*-*-%d-*-*-*-*-*-*-*",       0.833},
  {"-*-times-bold-i-*-*-%d-*-*-*-*-*-*-*",       0.778},
  {"-*-courier-medium-r-*-*-%d-*-*-*-*-*-*-*",   0.600},
  {"-*-courier-medium-o-*-*-%d-*-*-*-*-*-*-*",   0.600},
  {"-*-courier-bold-r-*-*-%d-*-*-*-*-*-*-*",     0.600},
  {"-*-courier-bold-o-*-*-%d-*-*-*-*-*-*-*",     0.600},
  {"-*-courier-medium-r-*-*-%d-*-*-*-*-*-*-*",   0.600},
  {"-*-courier-medium-o-*-*-%d-*-*-*-*-*-*-*",   0.600},
  {"-*-courier-bold-r-*-*-%d-*-*-*-*-*-*-*",     0.600},
  {"-*-courier-bold-o-*-*-%d-*-*-*-*-*-*-*",     0.600}
};

//------------------------------------------------------------------------
// Constructed characters
//------------------------------------------------------------------------

#define firstMultiChar 0x102
#define lastMultiChar 0x106

// custom-built chars:
//   100 bullet
//   101 trademark

// chars built from multiple normal chars:
static char *multiChars[] = {
  "fi",				// 102 fi
  "fl",				// 103 fl
  "...",			// 104 ellipsis
  "``",				// 105 quotedblleft
  "''"				// 106 quotedblright
};

//------------------------------------------------------------------------
// XOutputFontCache
//------------------------------------------------------------------------

XOutputFontCache::XOutputFontCache(Display *display1) {
  display = display1;
  numFonts = 0;
}

XOutputFontCache::~XOutputFontCache() {
  int i;

  for (i = 0; i < numFonts; ++i)
    XFreeFont(display, fonts[i].font);
}

XFontStruct *XOutputFontCache::getFont(char *name) {
  int i, j;
  XOutputFont font;

  // is it the most recently used font?
  if (!strcmp(fonts[0].name, name))
    return fonts[0].font;

  // is it in the cache?
  for (i = 1; i < numFonts; ++i) {
    if (!strcmp(fonts[i].name, name)) {
      font = fonts[i];
      for (j = i; j > 0; --j)
	fonts[j] = fonts[j-1];
      fonts[0] = font;
      return font.font;
    }
  }

  // need to load it
  font.font = XLoadQueryFont(display, name);
  if (font.font) {
    if (numFonts == fontCacheSize) {
      XFreeFont(display, fonts[fontCacheSize - 1].font);
      --numFonts;
    }
    for (j = numFonts; j > 0; --j)
      fonts[j] = fonts[j-1];
    strcpy(fonts[0].name, name);
    fonts[0].font = font.font;
    ++numFonts;
  }
  return font.font;
}

//------------------------------------------------------------------------
// XOutputDev
//------------------------------------------------------------------------

XOutputDev::XOutputDev(LTKWindow *win1) {
  XGCValues gcValues;
  XColor xcolor;
  Colormap colormap;
  int r, g, b, n;
  Boolean ok;

  // get pointer to X stuff
  win = win1;
  canvas = (LTKScrollingCanvas *)win->findWidget("canvas");
  display = win->getDisplay();
  screenNum = win->getScreenNum();
  pixmap = canvas->getPixmap();

  // allocate a color cube
  colormap = DefaultColormap(display, screenNum);
  for (numColors = maxColorCube; numColors >= 2; --numColors) {
    ok = true;
    n = 0;
    for (r = 0; r < numColors && ok; ++r) {
      for (g = 0; g < numColors && ok; ++g) {
	for (b = 0; b < numColors && ok; ++b) {
	  if (r == numColors - 1 && g == numColors - 1 && b == numColors -1) {
	    colors[n++] = WhitePixel(display, screenNum);
	  } else {
	    xcolor.red = (r * 65535) / (numColors - 1);
	    xcolor.green = (g * 65535) / (numColors - 1);
	    xcolor.blue = (b * 65535) / (numColors - 1);
	    if (XAllocColor(display, colormap, &xcolor))
	      colors[n++] = xcolor.pixel;
	    else
	      ok = false;
	  }
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

  // set up the font cache
  fontCache = new XOutputFontCache(display);

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

  s = new XOutputState;
  s->strokeGC = XCreateGC(display, pixmap, 0, &values);
  XCopyGC(display, strokeGC, 0xffffffff, s->strokeGC);
  s->fillGC = XCreateGC(display, pixmap, 0, &values);
  XCopyGC(display, fillGC, 0xffffffff, s->fillGC);
  s->clipRegion = XCreateRegion();
  XUnionRegion(clipRegion, s->clipRegion, s->clipRegion);
  s->next = save;
  save = s;
}

void XOutputDev::restoreState(GfxState *state) {
  XOutputState *s;

  if (save) {
    s = save;
    save = save->next;
    XFreeGC(display, strokeGC);
    strokeGC = s->strokeGC;
    XFreeGC(display, fillGC);
    fillGC = s->fillGC;
    XDestroyRegion(clipRegion);
    clipRegion = s->clipRegion;
    XSetRegion(display, strokeGC, clipRegion);
    XSetRegion(display, fillGC, clipRegion);
    delete s;
  }
}

void XOutputDev::updateAll(GfxState *state) {
  updateLineAttrs(state, true);
  updateMiterLimit(state);
  updateFillColor(state);
  updateStrokeColor(state);
  updateFont(state);
}

void XOutputDev::updateLineDash(GfxState *state){
  updateLineAttrs(state, true);
}

void XOutputDev::updateLineJoin(GfxState *state) {
  updateLineAttrs(state, false);
}

void XOutputDev::updateLineCap(GfxState *state) {
  updateLineAttrs(state, false);
}

// unimplemented
void XOutputDev::updateMiterLimit(GfxState *state) {
}

void XOutputDev::updateLineWidth(GfxState *state) {
  updateLineAttrs(state, false);
}

void XOutputDev::updateLineAttrs(GfxState *state, Boolean updateDash) {
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
  XSetLineAttributes(display, strokeGC, (int)(width + 0.5),
		     dashLength > 0 ? LineOnOffDash : LineSolid,
		     cap, join);
  if (updateDash && dashLength > 0) {
    if (dashLength > 20)
      dashLength = 20;
    for (i = 0; i < dashLength; ++i) {
      dashList[i] = (int)(state->transformWidth(dashPattern[i]) + 0.5);
      if (dashList[i] == 0)
	dashList[i] = 1;
    }
    XSetDashes(display, strokeGC, (int)dashStart, dashList, dashLength);
  }
}

void XOutputDev::updateFillColor(GfxState *state) {
  XSetForeground(display, fillGC, findColor(state->getFillColor()));
}

void XOutputDev::updateStrokeColor(GfxState *state) {
  XSetForeground(display, strokeGC, findColor(state->getStrokeColor()));
}

// Note: if real font is substantially narrower than substituted
// font, the size is reduced accordingly.
void XOutputDev::updateFont(GfxState *state) {
  FontMapEntry *p;
  String *pdfFont;
  char fontName[100];
  double size;
  int index;
  double w1, w2;

  if (!state->getFont())
    return;
  gFont = state->getFont();
  pdfFont = gFont->getName();
  size = state->getTransformedFontSize();
  if (pdfFont) {
    for (p = fontMap; p->pdfFont; ++p) {
      if (!pdfFont->cmp(p->pdfFont))
	break;
    }
  } else {
    p = NULL;
  }
  if (p && p->pdfFont) {
    sprintf(fontName, p->xFont, (int)size);
  } else {
    if (gFont->isFixedWidth())
      index = 8;
    else if (gFont->isSerif())
      index = 4;
    else
      index = 0;
    if (gFont->isBold())
      index += 2;
    if (gFont->isItalic())
      index += 1;
    w1 = gFont->getWidth('m');
    w2 = fontSubst[index].mWidth;
    if (w1 < 0.9 * w2)
      size *= w1 / (0.9 * w2);
    sprintf(fontName, fontSubst[index].xFont, (int)size);
  }
  font = fontCache->getFont(fontName);
  if (font) {
    XSetFont(display, fillGC, font->fid);
    XSetFont(display, strokeGC, font->fid);
  } else {
    error(0, "Failed to open font: '%s'", fontName);
    font = None;
  }
  
  isoMap = gFont->getISOMap();
  revISOMap = gFont->getReverseISOMap();
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
    points = (XPoint *)smalloc(m * sizeof(XPoint));

  // draw the lines
  for (i = 0; i < n; ++i) {
    subpath = path->getSubpath(i);
    m = subpath->getNumPoints();
    for (j = 0; j < m; ++j) {
      state->transform(subpath->getX(j), subpath->getY(j), &x, &y);
      points[j].x = (int)x;
      points[j].y = (int)y;
    }
    XDrawLines(display, pixmap, strokeGC, points, m, CoordModeOrigin);
  }

  // free points
  if (points != tmpPoints)
    sfree(points);
}

void XOutputDev::fill(GfxState *state) {
  doFill(state, WindingRule);
}

void XOutputDev::eoFill(GfxState *state) {
  doFill(state, EvenOddRule);
}

// This contains a kludge to deal with fills with multiple subpaths.
// First, it divides up the subpaths into non-overlappign polygons by
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
    points = (XPoint *)smalloc(m * sizeof(XPoint));
  if (n < numTmpSubpaths)
    lengths = tmpLengths;
  else
    lengths = (int *)smalloc(n * sizeof(int));

  // allocate bounding rectangles array
  if (n < numTmpSubpaths)
    rects = tmpRects;
  else
    rects = (BoundingRect *)smalloc(n * sizeof(BoundingRect));

  // transform points
  k = 0;
  for (i = 0; i < n; ++i) {
    // do one subpath
    subpath = path->getSubpath(i);
    m = subpath->getNumPoints();
    for (j = 0; j < m; ++j) {

      // transform a point
      state->transform(subpath->getX(j), subpath->getY(j), &x, &y);
      points[k+j].x = x1 = (int)x;
      points[k+j].y = y1 = (int)y;

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
    sfree(points);
  if (lengths != tmpLengths)
    sfree(lengths);
  if (rects != tmpRects)
    sfree(rects);
}

#if 0
void XOutputDev::doFill(GfxState *state, int rule) {
  XPoint *points;
  GfxPath *path;
  GfxSubpath *subpath;
  BoundsRect *rects;
  double x, y;
  int x1, y1;
  int n, m, i, j, k;

  // set fill rule
  XSetFillRule(display, fillGC, rule);

  // transform points
  path = state->getPath();
  n = path->getNumSubpaths();
  m = 0;
  for (i = 0; i < n; ++i)
    m += path->getSubpath(i)->getNumPoints();
  points = (XPoint *)smalloc(m * sizeof(XPoint));
  rects = NULL;
  if (n > 1)
    rects = (BoundsRect *)smalloc(n * sizeof(BoundsRect));
  k = 0;
printf("fill polygon:\n");
  for (i = 0; i < n; ++i) {
printf("  subpath %d:\n", i);
    subpath = path->getSubpath(i);
    for (j = 0; j < subpath->getNumPoints(); ++j) {
      state->transform(subpath->getX(j), subpath->getY(j), &x, &y);
      points[k].x = x1 = (int)x;
      points[k].y = y1 = (int)y;
printf("    point %d: %g,%g (%d,%d)\n", j, subpath->getX(j), subpath->getY(j), x1, y1);
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
      ++k;
    }
  }

  // only one subpath
  if (n == 1) {
    XFillPolygon(display, pixmap, fillGC, points, m, Complex, CoordModeOrigin);

  // multiple subpaths
  } else {
    j = 0;
    for (i = 0; i < n; ++i) {
      m = path->getSubpath(i)->getNumPoints();
      XFillPolygon(display, pixmap, fillGC, points + j, m,
		   Complex, CoordModeOrigin);
      j += m;
    }
/*
    XFillPolygon(display, pixmap, fillGC, points, m, Complex, CoordModeOrigin);
*/
    sfree(rects);
  }

  // free points array
  sfree(points);
}
#endif

void XOutputDev::clip(GfxState *state) {
  XPoint *points;
  int n;
  Region region;

  points = pathPoints(state, &n);
  region = XPolygonRegion(points, n, WindingRule);
  sfree(points);
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
  sfree(points);
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
  points = (XPoint *)smalloc(m * sizeof(XPoint));
  k = 0;
  for (i = 0; i < n; ++i) {
    subpath = path->getSubpath(i);
    for (j = 0; j < subpath->getNumPoints(); ++j) {
      state->transform(subpath->getX(j), subpath->getY(j), &x, &y);
      points[k].x = (int)x;
      points[k].y = (int)y;
      ++k;
    }
  }
  *numPoints = m;
  return points;
}

void XOutputDev::drawChar(GfxState *state, double x, double y, ushort c) {
  ushort c1;
  char buf;
  char *p;
  int n, i;
  double x1, y1;
  double tx;

  if (font == None)
    return;
  state->transform(x, y, &x1, &y1);
  c1 = isoMap[c];
  if (c1 == 0) {
//~    error(0, "unknown char: %03x", gFont->getPDFMap()[c]);
  } else if (c1 < 0x100) {
    buf = (char)c1;
    XDrawString(display, pixmap,
		(state->getRender() & 1) ? strokeGC : fillGC,
		(int)x1, (int)y1, &buf, 1);
  } else if (c1 < firstMultiChar) {
    switch (c1) {
    case 0x100: // bullet
      tx = 0.25 * state->getTransformedFontSize() * gFont->getWidth(c);
      XFillRectangle(display, pixmap,
		     (state->getRender() & 1) ? strokeGC : fillGC,
		     (int)(x1 + tx), (int)(y1 - 0.4 * font->ascent - tx),
		     (int)(2 * tx), (int)(2 * tx));
      break;
    case 0x101: // trademark
//~      tx = state->getTransformedFontSize() *
//~           (gFont->getWidth(c) - gFont->getWidth(revISOMap['M']));
      tx = 0.9 * state->getTransformedFontSize() *
           gFont->getWidth(revISOMap['T']);
      y1 -= 0.33 * (double)font->ascent;
      buf = 'T';
      XDrawString(display, pixmap,
		  (state->getRender() & 1) ? strokeGC : fillGC,
		  (int)x1, (int)y1, &buf, 1);
      x1 += tx;
      buf = 'M';
      XDrawString(display, pixmap,
		  (state->getRender() & 1) ? strokeGC : fillGC,
		  (int)x1, (int)y1, &buf, 1);
      break;
    }
  } else if (c1 <= lastMultiChar) {
    p = multiChars[c1 - firstMultiChar];
    n = strlen(p);
    tx = gFont->getWidth(c);
    for (i = 1; i < n; ++i)
      tx -= gFont->getWidth(revISOMap[p[i]]);
    tx = tx * state->getTransformedFontSize() / (double)(n - 1);
    for (i = 0; i < n; ++i) {
      XDrawString(display, pixmap,
		  (state->getRender() & 1) ? strokeGC : fillGC,
		  (int)x1, (int)y1, p + i, 1);
      x1 += tx;
    }
  }
}

void XOutputDev::drawImageMask(GfxState *state, Stream *str,
			       int width, int height, Boolean invert) {
  XImage *image;
  int x0, y0;			// top left corner of image
  int w0, h0, w1, h1;		// size of image
  uint depth;
  double xt, yt, wt, ht;
  Boolean rotate, xFlip, yFlip;
  int x, y;
  int ix, iy;
  int px1, px2, qx, dx;
  int py1, py2, qy, dy;
  ulong color;
  ulong buf;
  int bits;
  int pix;
  int i, j;

  // get image position size
  state->transform(0, 0, &xt, &yt);
  state->transformDelta(1, 1, &wt, &ht);
  if (wt > 0) {
    x0 = (int)xt;
    w0 = (int)wt;
  } else {
    x0 = (int)(xt + wt);
    w0 = (int)-wt;
  }
  if (ht > 0) {
    y0 = (int)yt;
    h0 = (int)ht;
  } else {
    y0 = (int)(yt + ht);
    h0 = (int)-ht;
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

  // allocate XImage
  depth = DefaultDepth(display, screenNum);
  if (x0 < 0 || x0 + w0 > canvas->getRealWidth() ||
      y0 < 0 || y0 + h0 > canvas->getRealHeight()) {
    error(0, "Badly placed image mask");
    return;
  }
  image = XGetImage(display, pixmap, x0, y0, w0, h0,
		    (1 << depth) - 1, ZPixmap);

  // Bresenham parameters
  px1 = (w1 + 1) / width;
  px2 = (w1 + 1) - px1 * width;
  py1 = (h1 + 1) / height;
  py2 = (h1 + 1) - py1 * height;

  // first line (column)
  y = yFlip ? h1 - 1 : 0;
  qy = 0;

  // read image
  for (i = 0; i < height; ++i) {

    // clear stream buffer at start of each line
    bits = 0;
    buf = 0;

    // vertical (horizontal) Bresenham
    dy = py1;
    if ((qy += py2) > height) {
      ++dy;
      qy -= height;
    }

    // first column (line)
    x = xFlip ? w1 - 1 : 0;
    qx = 0;

    // read one line (column)
    for (j = 0; j < width; ++j) {

      // horizontal (vertical) Bresenham
      dx = px1;
      if ((qx += px2) > width) {
	++dx;
	qx -= width;
      }

      // get pixel value
      if (bits == 0) {
	buf = str->getChar();
	bits = 8;
      }
      pix = (buf >> (bits - 1)) & 1;
      if (invert)
	pix = 1 - pix;
      --bits;

      // draw image pixel
      if (pix == 0 && dx > 0 && dy > 0) {
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
      if (xFlip)
	x -= dx;
      else
	x += dx;
    }
    if (yFlip)
      y -= dy;
    else
      y += dy;
  }

  XPutImage(display, pixmap, fillGC, image, 0, 0, x0, y0, w0, h0);
  XDestroyImage(image);
}

void XOutputDev::drawImage(GfxState *state, Stream *str, int width,
			   int height, GfxColorSpace *colorSpace) {
  XImage *image;
  int x0, y0;			// top left corner of image
  int w0, h0, w1, h1;		// size of image
  uint depth;
  int bytes;
  char *data;
  double xt, yt, wt, ht;
  Boolean rotate, xFlip, yFlip;
  int x, y;
  int ix, iy;
  int px1, px2, qx, dx;
  int py1, py2, qy, dy;
  int pix[4];
  ulong color;
  ulong buf;
  int bits;
  int nComps, nBits;
  ulong lookup[256];
  Boolean shortcut;
  int i, j, k;

  // get image position and size
  state->transform(0, 0, &xt, &yt);
  state->transformDelta(1, 1, &wt, &ht);
  if (wt > 0) {
    x0 = (int)xt;
    w0 = (int)wt;
  } else {
    x0 = (int)(xt + wt);
    w0 = (int)-wt;
  }
  if (ht > 0) {
    y0 = (int)yt;
    h0 = (int)ht;
  } else {
    y0 = (int)(yt + ht);
    h0 = (int)-ht;
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
  nBits = colorSpace->getBits();

  // check for tiny (zero width or height) images
  if (w0 == 0 || h0 == 0) {
    k = height * ((width * nComps * nBits + 7) / 8);
    for (i = 0; i < k; ++i)
      str->getChar();
    return;
  }

  // allocate XImage
  depth = DefaultDepth(display, screenNum);
  bytes = (w0 * depth + 7) >> 3;
  data = (char *)smalloc(h0 * bytes);
  image = XCreateImage(display, DefaultVisual(display, screenNum),
		       depth, ZPixmap, 0, data, w0, h0, 8, 0);

  // pre-decode lookup table, if possible
  if (colorSpace->isIndexed() && nBits <= 8) {
    shortcut = true;
    for (i = 0; i < (1 << nBits); ++i) {
      pix[0] = i;
      lookup[i] = findColor(pix, colorSpace);
    }
  } else {
    shortcut = false;
  }

  // Bresenham parameters
  px1 = (w1 + 1) / width;
  px2 = (w1 + 1) - px1 * width;
  py1 = (h1 + 1) / height;
  py2 = (h1 + 1) - py1 * height;

  // first line (column)
  y = yFlip ? h1 - 1 : 0;
  qy = 0;

  // read image
  for (i = 0; i < height; ++i) {

    // clear stream buffer at start of each line
    bits = 0;
    buf = 0;

    // vertical (horizontal) Bresenham
    dy = py1;
    if ((qy += py2) > height) {
      ++dy;
      qy -= height;
    }

    // first column (line)
    x = xFlip ? w1 - 1 : 0;
    qx = 0;

    // read one line
    for (j = 0; j < width; ++j) {

      // horizontal (vertical) Bresenham
      dx = px1;
      if ((qx += px2) > width) {
	++dx;
	qx -= width;
      }

      // get pixel value
      for (k = 0; k < nComps; ++k) {
	while (bits < nBits) {
	  buf = (buf << 8) | (str->getChar() & 0xff);
	  bits += 8;
	}
	pix[k] = (buf >> (bits - nBits)) & ((1 << nBits) - 1);
	bits -= nBits;
      }

      // draw image pixel
      if (dx > 0 && dy > 0) {
	if (shortcut)
	  color = lookup[pix[0]];
	else
	  color = findColor(pix, colorSpace);
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
      if (xFlip)
	x -= dx;
      else
	x += dx;
    }
    if (yFlip)
      y -= dy;
    else
      y += dy;
  }

  XPutImage(display, pixmap, fillGC, image, 0, 0, x0, y0, w0, h0);
  XDestroyImage(image);
}

unsigned long XOutputDev::findColor(GfxColor *color) {
  int r, g, b;
  double gray;
  unsigned long pixel;

  if (numColors == 1) {
    gray = color->getGray();
    if (gray < 0.01)
      pixel = colors[0];
    else
      pixel = colors[1];
  } else {
    r = (int)(color->getR() * (numColors - 1) + 0.5);
    g = (int)(color->getG() * (numColors - 1) + 0.5);
    b = (int)(color->getB() * (numColors - 1) + 0.5);
    pixel = colors[(r * numColors + g) * numColors + b];
  }
  return pixel;
}

unsigned long XOutputDev::findColor(int x[4], GfxColorSpace *colorSpace) {
  uchar r, g, b;
  int r1, g1, b1;
  uchar gray;
  unsigned long pixel;

  if (numColors == 1) {
    colorSpace->getGray(x, &gray);
    if (gray < 5)
      pixel = colors[0];
    else
      pixel = colors[1];
  } else {
    colorSpace->getRGB(x, &r, &g, &b);
    r1 = (r * (numColors - 1) + 128) / 255;
    g1 = (g * (numColors - 1) + 128) / 255;
    b1 = (b * (numColors - 1) + 128) / 255;
    pixel = colors[(r1 * numColors + g1) * numColors + b1];
  }
  return pixel;
}
