//========================================================================
//
// GfxState.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stddef.h>
#include <math.h>
#include <string.h> // for memcpy()
#include <gmem.h>
#include "Object.h"
#include "GfxState.h"

//------------------------------------------------------------------------
// GfxColor
//------------------------------------------------------------------------

void GfxColor::setCMYK(double c, double m, double y, double k) {
  if ((r = 1 - (c + k)) < 0)
    r = 0;
  if ((g = 1 - (m + k)) < 0)
    g = 0;
  if ((b = 1 - (y + k)) < 0)
    b = 0;
}

//------------------------------------------------------------------------
// GfxColorSpace
//------------------------------------------------------------------------

GfxColorSpace::GfxColorSpace() {
  mode = colorGray;
  indexed = gFalse;
  bits = 1;
  numComponents = 1;
  lookupComponents = 1;
  lookup = (Guchar (*)[4])gmalloc((1 << bits) * 4 * sizeof(Guchar));
  ok = gTrue;
}

GfxColorSpace::GfxColorSpace(int bits1, Object *colorSpace, Object *decode) {
  Object obj, obj2;
  double decodeLow[4], decodeHigh[4];
  int decodeComps;
  Guchar (*palette)[4];
  int indexHigh;
  char *s;
  int x;
  int i, j, k;

  ok = gTrue;
  bits = bits1;
  lookup = NULL;
  palette = NULL;

  // get decode map
  if (decode->isNull()) {
    decodeComps = 0;
  } else if (decode->isArray()) {
    decodeComps = decode->arrayGetLength() / 2;
    for (i = 0; i < decodeComps; ++i) {
      decode->arrayGet(2*i, &obj);
      if (!obj.isNum())
	goto err2;
      decodeLow[i] = obj.getNum();
      obj.free();
      decode->arrayGet(2*i+1, &obj);
      if (!obj.isNum())
	goto err2;
      decodeHigh[i] = obj.getNum();
      obj.free();
    }
  } else {
    goto err1;
  }

  // get mode
  indexed = gFalse;
  if (colorSpace->isName("DeviceGray") || colorSpace->isName("G")) {
    mode = colorGray;
    numComponents = lookupComponents = 1;
  } else if (colorSpace->isName("DeviceRGB") || colorSpace->isName("RGB")) {
    mode = colorRGB;
    numComponents = lookupComponents = 3;
  } else if (colorSpace->isName("DeviceCMYK") || colorSpace->isName("CMYK")) {
    mode = colorCMYK;
    numComponents = lookupComponents = 4;
  } else if (colorSpace->isArray()) {
    colorSpace->arrayGet(0, &obj);
    if (obj.isName("DeviceGray") || obj.isName("G") ||
	obj.isName("CalGray")) {
      mode = colorGray;
      numComponents = lookupComponents = 1;
    } else if (obj.isName("DeviceRGB") || obj.isName("RGB") ||
	       obj.isName("CalRGB")) {
      mode = colorRGB;
      numComponents = lookupComponents = 3;
    } else if (obj.isName("DeviceCMYK") || obj.isName("CMYK")) {
      mode = colorCMYK;
      numComponents = lookupComponents = 4;
    } else if (obj.isName("Indexed") || obj.isName("I")) {
      indexed = gTrue;
      numComponents = 1;
    } else {
      goto err2;
    }
    obj.free();
  }

  // indexed decoding
  if (indexed) {
    if (decodeComps == 0) {
      decodeLow[0] = 0;
      decodeHigh[0] = (1 << bits) - 1;
    } else if (decodeComps != numComponents) {
      goto err1;
    }
    colorSpace->arrayGet(1, &obj);
    if (obj.isName("DeviceGray") || obj.isName("G")) {
      mode = colorGray;
      lookupComponents = 1;
    } else if (obj.isName("DeviceRGB") || obj.isName("RGB")) {
      mode = colorRGB;
      lookupComponents = 3;
    } else if (obj.isName("DeviceCMYK") || obj.isName("CMYK")) {
      mode = colorCMYK;
      lookupComponents = 4;
    } else if (obj.isArray()) {
      obj.arrayGet(0, &obj2);
      if (obj.isName("CalGray")) {
	mode = colorGray;
	lookupComponents = 1;
      } else if (obj.isName("CalRGB")) {
	mode = colorRGB;
	lookupComponents = 3;
      } else {
	obj2.free();
	goto err2;
      }
      obj2.free();
    } else {
      goto err2;
    }
    obj.free();
    colorSpace->arrayGet(2, &obj);
    if (!obj.isInt())
      goto err2;
    indexHigh = obj.getInt();
    obj.free();
    palette = (Guchar (*)[4])gmalloc((indexHigh + 1) * 4 * sizeof(Guchar));
    colorSpace->arrayGet(3, &obj);
    if (obj.isStream()) {
      obj.streamReset();
      for (i = 0; i <= indexHigh; ++i) {
	for (j = 0; j < lookupComponents; ++j) {
	  if ((x = obj.streamGetChar()) == EOF)
	    goto err3;
	  palette[i][j] = (Guchar)x;
	}
      }
    } else if (obj.isString()) {
      s = obj.getString()->getCString();
      for (i = 0; i <= indexHigh; ++i)
	for (j = 0; j < lookupComponents; ++j)
	  palette[i][j] = (Guchar)*s++;
    } else {
      goto err3;
    }
    obj.free();
    lookup = (Guchar (*)[4])gmalloc((1 << bits) * 4 * sizeof(Guchar));
    for (i = 0; i < (1 << bits); ++i) {
      k = (int)(decodeLow[0] + i * (decodeHigh[0] - decodeLow[0]) /
	                           ((1 << bits) - 1) + 0.5);
      switch (mode) {
      case colorGray:
	lookup[i][0] = lookup[i][1] = lookup[i][2] = palette[k][0];
	break;
      case colorCMYK:
	x = palette[k][0] + palette[k][3];
	lookup[i][0] = (x > 255) ? 0 : 255 - x;
	x = palette[k][1] + palette[k][3];
	lookup[i][1] = (x > 255) ? 0 : 255 - x;
	x = palette[k][2] + palette[k][3];
	lookup[i][2] = (x > 255) ? 0 : 255 - x;
	break;
      case colorRGB:
	lookup[i][0] = palette[k][0];
	lookup[i][1] = palette[k][1];
	lookup[i][2] = palette[k][2];
	break;
      }
    }
    gfree(palette);

  // non-indexed decoding
  } else {
    if (decodeComps == 0) {
      for (i = 0; i < numComponents; ++i) {
	decodeLow[i] = 0;
	decodeHigh[i] = 1;
      }
    } else if (decodeComps != numComponents) {
      goto err1;
    }
    lookup = (Guchar (*)[4])gmalloc((1 << bits) * 4 * sizeof(Guchar));
    for (i = 0; i < (1 << bits); ++i) {
      for (j = 0; j < lookupComponents; ++j) {
	lookup[i][j] = (Guchar)(255.0 * (decodeLow[j] +
					 (double)i *
					 (decodeHigh[j] - decodeLow[j]) /
					 (double)((1 << bits) - 1)));
      }
    }
  }

  return;

 err3:
  gfree(palette);
 err2:
  obj.free();
 err1:
  ok = gFalse;
}

GfxColorSpace::~GfxColorSpace() {
  gfree(lookup);
}

GfxColorSpace::GfxColorSpace(GfxColorSpace *colorSpace) {
  int size;

  memcpy(this, colorSpace, sizeof(GfxColorSpace));
  size = (1 << bits) * 4 * sizeof(Guchar);
  lookup = (Guchar (*)[4])gmalloc(size);
  memcpy(lookup, colorSpace->lookup, size);
}

void GfxColorSpace::getGray(Guchar x[4], Guchar *gray) {
  Guchar *p;

  if (indexed) {
    p = lookup[x[0]];
    *gray = (Guchar)(0.299 * p[0] + 0.587 * p[1] + 0.114 * p[2]);
  } else {
    switch (mode) {
    case colorGray:
      *gray = lookup[x[0]][0];
      break;
    case colorCMYK:
      *gray = 255 - (Guchar)(lookup[x[3]][3] -
			     0.299 * lookup[x[0]][0] -
			     0.587 * lookup[x[1]][1] -
			     0.114 * lookup[x[2]][2]);
      break;
    case colorRGB:
      *gray = (Guchar)(0.299 * lookup[x[0]][0] +
		       0.587 * lookup[x[1]][1] +
		       0.114 * lookup[x[2]][2]);
      break;
    }
  }
}

void GfxColorSpace::getRGB(Guchar x[4], Guchar *r, Guchar *g, Guchar *b) {
  Guchar *p;
  int t;

  if (indexed) {
    p = lookup[x[0]];
    *r = p[0];
    *g = p[1];
    *b = p[2];
  } else {
    switch (mode) {
    case colorGray:
      *r = *g = *b = lookup[x[0]][0];
      break;
    case colorCMYK:
      t = lookup[x[0]][0] + lookup[x[3]][3];
      *r = (t > 255) ? 0 : 255 - t;
      t = lookup[x[1]][1] + lookup[x[3]][3];
      *g = (t > 255) ? 0 : 255 - t;
      t = lookup[x[2]][2] + lookup[x[3]][3];
      *b = (t > 255) ? 0 : 255 - t;
      break;
    case colorRGB:
      *r = lookup[x[0]][0];
      *g = lookup[x[1]][1];
      *b = lookup[x[2]][2];
      break;
    }
  }
}

void GfxColorSpace::getColor(double x[4], GfxColor *color) {
  Guchar *p;

  if (indexed) {
    p = lookup[(int)x[0]];
    color->setRGB(p[0] / 255.0, p[1] / 255.0, p[2] / 255.0);
  } else {
    switch (mode) {
    case colorGray:
      color->setGray(x[0]);
      break;
    case colorCMYK:
      color->setCMYK(x[0], x[1], x[2], x[3]);
      break;
    case colorRGB:
      color->setRGB(x[0], x[1], x[2]);
      break;
    }
  }
}

//------------------------------------------------------------------------
// GfxSubpath and GfxPath
//------------------------------------------------------------------------

GfxSubpath::GfxSubpath(double x1, double y1) {
  size = 16;
  x = (double *)gmalloc(size * sizeof(double));
  y = (double *)gmalloc(size * sizeof(double));
  curve = (GBool *)gmalloc(size * sizeof(GBool));
  n = 1;
  x[0] = x1;
  y[0] = y1;
  curve[0] = gFalse;
}

GfxSubpath::~GfxSubpath() {
  gfree(x);
  gfree(y);
  gfree(curve);
}

// Used for copy().
GfxSubpath::GfxSubpath(GfxSubpath *subpath) {
  size = subpath->size;
  n = subpath->n;
  x = (double *)gmalloc(size * sizeof(double));
  y = (double *)gmalloc(size * sizeof(double));
  curve = (GBool *)gmalloc(size * sizeof(GBool));
  memcpy(x, subpath->x, n * sizeof(double));
  memcpy(y, subpath->y, n * sizeof(double));
  memcpy(curve, subpath->curve, n * sizeof(GBool));
}

void GfxSubpath::lineTo(double x1, double y1) {
  if (n >= size) {
    size += 16;
    x = (double *)grealloc(x, size * sizeof(double));
    y = (double *)grealloc(y, size * sizeof(double));
    curve = (GBool *)grealloc(curve, size * sizeof(GBool));
  }
  x[n] = x1;
  y[n] = y1;
  curve[n] = gFalse;
  ++n;
}

void GfxSubpath::curveTo(double x1, double y1, double x2, double y2,
			 double x3, double y3) {
  if (n+3 > size) {
    size += 16;
    x = (double *)grealloc(x, size * sizeof(double));
    y = (double *)grealloc(y, size * sizeof(double));
    curve = (GBool *)grealloc(curve, size * sizeof(GBool));
  }
  x[n] = x1;
  y[n] = y1;
  x[n+1] = x2;
  y[n+1] = y2;
  x[n+2] = x3;
  y[n+2] = y3;
  curve[n] = curve[n+1] = gTrue;
  curve[n+2] = gFalse;
  n += 3;
}

GfxPath::GfxPath() {
  justMoved = gFalse;
  size = 16;
  n = 0;
  subpaths = (GfxSubpath **)gmalloc(size * sizeof(GfxSubpath *));
}

GfxPath::~GfxPath() {
  int i;

  for (i = 0; i < n; ++i)
    delete subpaths[i];
  gfree(subpaths);
}

// Used for copy().
GfxPath::GfxPath(GBool justMoved1, double firstX1, double firstY1,
		 GfxSubpath **subpaths1, int n1, int size1) {
  int i;

  justMoved = justMoved1;
  firstX = firstX1;
  firstY = firstY1;
  size = size1;
  n = n1;
  subpaths = (GfxSubpath **)gmalloc(size * sizeof(GfxSubpath *));
  for (i = 0; i < n; ++i)
    subpaths[i] = subpaths1[i]->copy();
}

void GfxPath::moveTo(double x, double y) {
  justMoved = gTrue;
  firstX = x;
  firstY = y;
}

void GfxPath::lineTo(double x, double y) {
  if (justMoved) {
    if (n >= size) {
      size += 16;
      subpaths = (GfxSubpath **)
	           grealloc(subpaths, size * sizeof(GfxSubpath *));
    }
    subpaths[n] = new GfxSubpath(firstX, firstY);
    ++n;
    justMoved = gFalse;
  }
  subpaths[n-1]->lineTo(x, y);
}

void GfxPath::curveTo(double x1, double y1, double x2, double y2,
	     double x3, double y3) {
  if (justMoved) {
    if (n >= size) {
      size += 16;
      subpaths = (GfxSubpath **)
	           grealloc(subpaths, size * sizeof(GfxSubpath *));
    }
    subpaths[n] = new GfxSubpath(firstX, firstY);
    ++n;
    justMoved = gFalse;
  }
  subpaths[n-1]->curveTo(x1, y1, x2, y2, x3, y3);
}


//------------------------------------------------------------------------
// GfxState
//------------------------------------------------------------------------

GfxState::GfxState(int dpi, int x1a, int y1a, int x2a, int y2a, int rotate,
		   GBool upsideDown) {
  double k;

  x1 = x1a;
  y1 = y1a;
  x2 = x2a;
  y2 = y2a;
  k = (double)dpi / 72.0;
  if (rotate == 90) {
    ctm[0] = 0;
    ctm[1] = upsideDown ? k : -k;
    ctm[2] = k;
    ctm[3] = 0;
    ctm[4] = -k * y1;
    ctm[5] = k * (upsideDown ? -x1 : x2);
    pageWidth = (int)(k * (y2 - y1));
    pageHeight = (int)(k * (x2 - x1));
  } else if (rotate == 180) {
    ctm[0] = -k;
    ctm[1] = 0;
    ctm[2] = 0;
    ctm[3] = upsideDown ? k : -k;
    ctm[4] = k * x2;
    ctm[5] = k * (upsideDown ? -y1 : y2);
    pageWidth = (int)(k * (x2 - x1));
    pageHeight = (int)(k * (y2 - y1));
  } else if (rotate == 270) {
    ctm[0] = 0;
    ctm[1] = upsideDown ? -k : k;
    ctm[2] = -k;
    ctm[3] = 0;
    ctm[4] = k * y2;
    ctm[5] = k * (upsideDown ? x2 : -x1);
    pageWidth = (int)(k * (y2 - y1));
    pageHeight = (int)(k * (x2 - x1));
  } else {
    ctm[0] = k;
    ctm[1] = 0;
    ctm[2] = 0;
    ctm[3] = upsideDown ? -k : k;
    ctm[4] = -k * x1;
    ctm[5] = k * (upsideDown ? y2 : -y1);
    pageWidth = (int)(k * (x2 - x1));
    pageHeight = (int)(k * (y2 - y1));
  }

  fillColorSpace = new GfxColorSpace();
  strokeColorSpace = new GfxColorSpace();
  fillColor.setGray(0);
  strokeColor.setGray(0);

  lineWidth = 1;
  lineDash = NULL;
  lineDashLength = 0;
  lineDashStart = 0;
  flatness = 0;
  lineJoin = 0;
  lineCap = 0;
  miterLimit = 10;

  font = NULL;
  fontSize = 0;
  textMat[0] = 1; textMat[1] = 0;
  textMat[2] = 0; textMat[3] = 1;
  textMat[4] = 0; textMat[5] = 0;
  charSpace = 0;
  wordSpace = 0;
  horizScaling = 1;
  leading = 0;
  rise = 0;
  render = 0;

  path = new GfxPath();
  curX = curY = 0;
  lineX = lineY = 0;

  saved = NULL;
}

GfxState::~GfxState() {
  if (fillColorSpace)
    delete fillColorSpace;
  if (strokeColorSpace)
    delete strokeColorSpace;
  gfree(lineDash);
  delete path;
  if (saved)
    delete saved;
}

// Used for copy();
GfxState::GfxState(GfxState *state) {
  memcpy(this, state, sizeof(GfxState));
  if (fillColorSpace)
    fillColorSpace = state->fillColorSpace->copy();
  if (strokeColorSpace)
    strokeColorSpace = state->strokeColorSpace->copy();
  if (lineDashLength > 0) {
    lineDash = (double *)gmalloc(lineDashLength * sizeof(double));
    memcpy(lineDash, state->lineDash, lineDashLength * sizeof(double));
  }
  path = state->path->copy();
  saved = NULL;
}

double GfxState::transformWidth(double w) {
  double x, y;

  x = ctm[0] + ctm[2];
  y = ctm[1] + ctm[3];
  return w * sqrt(0.5 * (x * x + y * y));
}

double GfxState::getTransformedFontSize() {
  double x1, y1, x2, y2;

  x1 = textMat[2] * fontSize;
  y1 = textMat[3] * fontSize;
  x2 = ctm[0] * x1 + ctm[2] * y1;
  y2 = ctm[1] * x1 + ctm[3] * y1;
  return sqrt(x2 * x2 + y2 * y2);
}

void GfxState::getFontTransMat(double *m11, double *m12,
			       double *m21, double *m22) {
  *m11 = (textMat[0] * ctm[0] + textMat[1] * ctm[2]) * fontSize;
  *m12 = (textMat[0] * ctm[1] + textMat[1] * ctm[3]) * fontSize;
  *m21 = (textMat[2] * ctm[0] + textMat[3] * ctm[2]) * fontSize;
  *m22 = (textMat[2] * ctm[1] + textMat[3] * ctm[3]) * fontSize;
}

void GfxState::concatCTM(double a, double b, double c,
			 double d, double e, double f) {
  double a1 = ctm[0];
  double b1 = ctm[1];
  double c1 = ctm[2];
  double d1 = ctm[3];

  ctm[0] = a * a1 + b * c1;
  ctm[1] = a * b1 + b * d1;
  ctm[2] = c * a1 + d * c1;
  ctm[3] = c * b1 + d * d1;
  ctm[4] = e * a1 + f * c1 + ctm[4];
  ctm[5] = e * b1 + f * d1 + ctm[5];
}

void GfxState::setFillColorSpace(GfxColorSpace *colorSpace) {
  if (fillColorSpace)
    delete fillColorSpace;
  fillColorSpace = colorSpace;
}

void GfxState::setStrokeColorSpace(GfxColorSpace *colorSpace) {
  if (strokeColorSpace)
    delete strokeColorSpace;
  strokeColorSpace = colorSpace;
}

void GfxState::setLineDash(double *dash, int length, double start) {
  if (lineDash)
    gfree(lineDash);
  lineDash = dash;
  lineDashLength = length;
  lineDashStart = start;
}

void GfxState::clearPath() {
  delete path;
  path = new GfxPath();
}

void GfxState::textShift(double tx) {
  double dx, dy;

  textTransformDelta(tx, 0, &dx, &dy);
  curX += dx;
  curY += dy;
}

GfxState *GfxState::save() {
  GfxState *newState;

  newState = copy();
  newState->saved = this;
  return newState;
}

GfxState *GfxState::restore() {
  GfxState *oldState;

  if (saved) {
    oldState = saved;
    saved = NULL;
    delete this;
  } else {
    oldState = this;
  }
  return oldState;
}
