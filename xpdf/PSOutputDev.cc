//========================================================================
//
// PSOutputDev.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <signal.h>
#include <GString.h>
#include "config.h"
#include "Object.h"
#include "Error.h"
#include "GfxState.h"
#include "GfxFont.h"
#include "Catalog.h"
#include "Page.h"
#include "PSOutputDev.h"

//------------------------------------------------------------------------
// PostScript prolog and setup
//------------------------------------------------------------------------

static char *prolog[] = {
  "%%BeginProlog",
  "% PDF special state",
  "/pdfDictSize 14 def",
  "/pdfStartPage {",
  "  pdfDictSize dict begin",
  "  /pdfFill [0] def",
  "  /pdfStroke [0] def",
  "  /pdfLastFill false def",
  "  /pdfLastStroke false def",
  "  /pdfTextMat [1 0 0 1 0 0] def",
  "  /pdfFontSize 0 def",
  "  /pdfCharSpacing 0 def",
  "  /pdfTextRender 0 def",
  "  /pdfTextRise 0 def",
  "  /pdfWordSpacing 0 def",
  "  /pdfHorizScaling 100 def",
  "} def",
  "/pdfEndPage { end } def",
  "/sCol { pdfLastStroke not {",
  "          pdfStroke aload length",
  "          1 eq { setgray } { setrgbcolor} ifelse",
  "          /pdfLastStroke true def /pdfLastFill false def",
  "        } if } def",
  "/fCol { pdfLastFill not {",
  "          pdfFill aload length",
  "          1 eq { setgray } { setrgbcolor } ifelse",
  "          /pdfLastFill true def /pdfLastStroke false def",
  "        } if } def",
  "% build a font",
  "/pdfMakeFont {",
  "  3 2 roll findfont",
  "  3 2 roll 1 matrix scale makefont",
  "  dup length dict begin",
  "    { 1 index /FID ne { def } { pop pop } ifelse } forall",
  "    /Encoding exch def",
  "    currentdict",
  "  end",
  "  definefont pop",
  "} def",
  "% graphics state operators",
  "/q { gsave pdfDictSize dict begin } def",
  "/Q { end grestore } def",
  "/cm { concat } def",
  "/d { setdash } def",
  "/i { setflat } def",
  "/j { setlinejoin } def",
  "/J { setlinecap } def",
  "/M { setmiterlimit } def",
  "/w { setlinewidth } def",
  "% color operators",
  "/g { dup 1 array astore /pdfFill exch def setgray",
  "    /pdfLastFill true def /pdfLastStroke false def } def",
  "/G { dup 1 array astore /pdfStroke exch def setgray",
  "     /pdfLastStroke true def /pdfLastFill false def } def",
  "/rg { 3 copy 3 array astore /pdfFill exch def setrgbcolor",
  "     /pdfLastFill true def /pdfLastStroke false def } def",
  "/RG { 3 copy 3 array astore /pdfStroke exch def setrgbcolor",
  "     /pdfLastStroke true def /pdfLastFill false def } def",
  "% path segment operators",
  "/m { moveto } def",
  "/l { lineto } def",
  "/c { curveto } def",
  "% path painting operators",
  "/S { sCol stroke } def",
  "/f { fCol fill } def",
  "/f* { fCol eofill } def",
  "% clipping operators",
  "/W { clip newpath } def",
  "/W* { eoclip newpath } def",
  "% text state operators",
  "/Tc { /pdfCharSpacing exch def } def",
  "/Tf { dup /pdfFontSize exch def",
  "      dup pdfHorizScaling 100 div mul exch matrix scale",
  "      pdfTextMat matrix concatmatrix dup 4 0 put dup 5 0 put",
  "      exch findfont exch makefont setfont } def",
  "/Tr { /pdfTextRender exch def } def",
  "/Ts { /pdfTextRise exch def } def",
  "/Tw { /pdfWordSpacing exch def } def",
  "/Tz { /pdfHorizScaling exch def } def",
  "% text positioning operators",
  "/Td { pdfTextMat transform moveto } def",
  "/Tm { /pdfTextMat exch def } def",
  "% text string operators",
  "/Tj { pdfTextRender 1 and 0 eq { fCol } { sCol } ifelse",
  "      0 pdfTextRise pdfTextMat dtransform rmoveto",
  "      pdfFontSize mul",
  "      1 index stringwidth pdfTextMat idtransform pop",
  "      sub 1 index length div",
  "      pdfWordSpacing 0 pdfTextMat dtransform 32",
  "      4 3 roll pdfCharSpacing add 0 pdfTextMat dtransform",
  "      6 5 roll awidthshow",
  "      0 pdfTextRise neg pdfTextMat dtransform rmoveto } def",
  "/TJm { pdfFontSize 0.001 mul mul neg 0",
  "       pdfTextMat dtransform rmoveto } def",
  "% image operators",
  "/pdfImBuf 100 string def",
  "/pdfIm {",
  "  image",
  "  { currentfile pdfImBuf readline",
  "    not { pop exit } if",
  "    (%-EOD-) eq { exit } if } loop",
  "} def",
  "/pdfImM {",
  "  imagemask",
  "  { currentfile pdfImBuf readline",
  "    not { pop exit } if",
  "    (%-EOD-) eq { exit } if } loop",
  "} def",
  "%%EndProlog",
  NULL
};

//------------------------------------------------------------------------
// Fonts
//------------------------------------------------------------------------

struct PSFont {
  char *name;			// PDF name
  char *psName;			// PostScript name
};

struct PSSubstFont {
  char *psName;			// PostScript name
  double mWidth;		// width of 'm' character
};

static PSFont psFonts[] = {
  {"Courier",               "Courier"},
  {"Courier-Bold",          "Courier-Bold"},
  {"Courier-Oblique",       "Courier-Bold"},
  {"Courier-BoldOblique",   "Courier-BoldOblique"},
  {"Helvetica",             "Helvetica"},
  {"Helvetica-Bold",        "Helvetica-Bold"},
  {"Helvetica-Oblique",     "Helvetica-Oblique"},
  {"Helvetica-BoldOblique", "Helvetica-BoldOblique"},
  {"Symbol",                "Symbol"},
  {"Times-Roman",           "Times-Roman"},
  {"Times-Bold",            "Times-Bold"},
  {"Times-Italic",          "Times-Italic"},
  {"Times-BoldItalic",      "Times-BoldItalic"},
  {"ZapfDingbats",          "ZapfDingbats"},
  {NULL}
};

static PSSubstFont psSubstFonts[] = {
  {"Helvetica",             0.833},
  {"Helvetica-Oblique",     0.833},
  {"Helvetica-Bold",        0.889},
  {"Helvetica-BoldOblique", 0.889},
  {"Times-Roman",           0.788},
  {"Times-Italic",          0.722},
  {"Times-Bold",            0.833},
  {"Times-BoldItalic",      0.778},
  {"Courier",               0.600},
  {"Courier-Oblique",       0.600},
  {"Courier-Bold",          0.600},
  {"Courier-BoldOblique",   0.600}
};

//------------------------------------------------------------------------
// PSOutputDev
//------------------------------------------------------------------------

PSOutputDev::PSOutputDev(char *fileName, Catalog *catalog,
			 int firstPage, int lastPage) {
  Dict *fontDict;
  GfxFontDict *gfxFontDict;
  GfxFont *font;
  Ref *ids;
  int numIDs;
  char **p;
  int pg, i, j, k;

  // open file or pipe
  ok = gTrue;
  if (!strcmp(fileName, "-")) {
    fileType = psStdout;
    f = stdout;
  } else if (fileName[0] == '|') {
    fileType = psPipe;
#ifdef NO_POPEN
    error(-1, "Print commands are not supported ('%s')", fileName);
    ok = gFalse;
    return;
#else
    signal(SIGPIPE, (void (*)(int))SIG_IGN);
    if (!(f = popen(fileName + 1, "w"))) {
      error(-1, "Couldn't run print command '%s'", fileName);
      ok = gFalse;
      return;
    }
#endif
  } else {
    fileType = psFile;
    if (!(f = fopen(fileName, "w"))) {
      error(-1, "Couldn't open PostScript file '%s'", fileName);
      ok = gFalse;
      return;
    }
  }

  // write header
  writePS("%%!PS-Adobe-3.0\n");
  writePS("%%%%Creator: xpdf/pdftops %s\n", xpdfVersion);
  writePS("%%%%Pages: %d\n", lastPage - firstPage + 1);
  writePS("%%%%EndComments\n");

  // write prolog
  for (p = prolog; *p; ++p)
    writePS("%s\n", *p);

  // write document setup
  writePS("%%%%BeginSetup\n");
  numIDs = 64;
  ids = (Ref *)gmalloc(numIDs * sizeof(Ref));
  j = 0;
  for (pg = firstPage; pg <= lastPage; ++pg) {
    fontDict = catalog->getPage(pg)->getFontDict();
    if (fontDict) {
      gfxFontDict = new GfxFontDict(fontDict);
      for (i = 0; i < gfxFontDict->getNumFonts(); ++i) {
	font = gfxFontDict->getFont(i);
	for (k = 0; k < j; ++k) {
	  if (ids[j].num == font->getID().num &&
	      ids[j].gen == font->getID().gen)
	    break;
	}
	if (k >= j) {
	  if (j >= numIDs) {
	    numIDs += 64;
	    ids = (Ref *)grealloc(ids, numIDs * sizeof(Ref));
	  }
	  ids[j++] = font->getID();
	  setupFont(font);
	}
      }
      delete gfxFontDict;
    }
  }
  writePS("%%%%EndSetup\n");

  // initialize sequential page number
  seqPage = 1;
}

PSOutputDev::~PSOutputDev() {
  if (f) {
    writePS("%%%%Trailer\n");
    writePS("%%%%EOF\n");
    if (fileType == psFile) {
      fclose(f);
    }
#ifndef NO_POPEN
    else if (fileType == psPipe) {
      pclose(f);
      signal(SIGPIPE, (void (*)(int))SIG_DFL);
    }
#endif
  }
}

void PSOutputDev::setupFont(GfxFont *font) {
  GString *name;
  char *psName;
  double scale;
  int i;

  name = font->getName();
  psName = NULL;
  scale = 1.0;
  if (name) {
    for (i = 0; psFonts[i].name; ++i) {
      if (name->cmp(psFonts[i].name) == 0) {
	psName = psFonts[i].psName;
	break;
      }
    }
  }
  if (!psName) {
    if (font->isFixedWidth())
      i = 8;
    else if (font->isSerif())
      i = 4;
    else
      i = 0;
    if (font->isBold())
      i += 2;
    if (font->isItalic())
      i += 1;
    psName = psSubstFonts[i].psName;
    scale = font->getWidth('m') / psSubstFonts[i].mWidth;
    if (scale < 0.1)
      scale = 1;
  }

  writePS("/F%d_%d /%s %g\n",
	  font->getID().num, font->getID().gen, psName, scale);
  for (i = 0; i < 256; ++i)
    writePS("%s/%s\n", (i == 0) ? "[ " : "  ", font->getCharName(i));
  writePS("]\n");
  writePS("pdfMakeFont\n");
}

void PSOutputDev::startPage(int pageNum, GfxState *state) {
  int x1, y1, x2, y2, width, height, t;
  double xScale, yScale;

  writePS("%%%%Page: %d %d\n", pageNum, seqPage);
  writePS("%%%%BeginPageSetup\n");
  writePS("pdfStartPage\n");

  // rotate, translate, and scale page
  x1 = state->getX1();
  y1 = state->getY1();
  x2 = state->getX2();
  y2 = state->getY2();
  width = x2 - x1;
  height = y2 - y1;
  if (width > height) {
    writePS("90 rotate\n");
    writePS("%d %d translate\n", -x1, -(y1 + paperWidth));
    t = width;
    width = height;
    height = t;
  } else {
    if (x1 != 0 || y1 != 0)
      writePS("%d %d translate\n", -x1, -y1);
  }
  if (width > paperWidth || height > paperHeight) {
    xScale = (double)paperWidth / (double)width;
    yScale = (double)paperHeight / (double)height;
    if (yScale < xScale)
      xScale = yScale;
    writePS("%0.4f %0.4f scale\n", xScale, xScale);
  }

  writePS("%%%%EndPageSetup\n");
  ++seqPage;
}

void PSOutputDev::endPage() {
  writePS("showpage\n");
  writePS("%%%%PageTrailer\n");
  writePS("pdfEndPage\n");
}

void PSOutputDev::saveState(GfxState *state) {
  writePS("q\n");
}

void PSOutputDev::restoreState(GfxState *state) {
  writePS("Q\n");
}

void PSOutputDev::updateCTM(GfxState *state, double m11, double m12,
			    double m21, double m22, double m31, double m32) {
  writePS("[%g %g %g %g %g %g] cm\n", m11, m12, m21, m22, m31, m32);
}

void PSOutputDev::updateLineDash(GfxState *state) {
  double *dash;
  double start;
  int length, i;

  state->getLineDash(&dash, &length, &start);
  writePS("[");
  for (i = 0; i < length; ++i)
    writePS("%g%s", dash[i], (i == length-1) ? "" : " ");
  writePS("] %g d\n", start);
}

void PSOutputDev::updateFlatness(GfxState *state) {
  writePS("%d i\n", state->getFlatness());
}

void PSOutputDev::updateLineJoin(GfxState *state) {
  writePS("%d j\n", state->getLineJoin());
}

void PSOutputDev::updateLineCap(GfxState *state) {
  writePS("%d J\n", state->getLineCap());
}

void PSOutputDev::updateMiterLimit(GfxState *state) {
  writePS("%g M\n", state->getMiterLimit());
}

void PSOutputDev::updateLineWidth(GfxState *state) {
  writePS("%g w\n", state->getLineWidth());
}

void PSOutputDev::updateFillColor(GfxState *state) {
  GfxColor *color;
  double r, g, b;

  color = state->getFillColor();
  r = color->getR();
  g = color->getG();
  b = color->getB();
  if (r == g && g == b)
    writePS("%g g\n", r);
  else
    writePS("%g %g %g rg\n", r, g, b);
}

void PSOutputDev::updateStrokeColor(GfxState *state) {
  GfxColor *color;
  double r, g, b;

  color = state->getFillColor();
  r = color->getR();
  g = color->getG();
  b = color->getB();
  if (r == g && g == b)
    writePS("%g G\n", r);
  else
    writePS("%g %g %g RG\n", r, g, b);
}

void PSOutputDev::updateFont(GfxState *state) {
  writePS("/F%d_%d %g Tf\n",
	  state->getFont()->getID().num, state->getFont()->getID().gen,
	  state->getFontSize());
}

void PSOutputDev::updateTextMat(GfxState *state) {
  double *mat;

  mat = state->getTextMat();
  writePS("[%g %g %g %g %g %g] Tm\n",
	  mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
}

void PSOutputDev::updateCharSpace(GfxState *state) {
  writePS("%g Tc\n", state->getCharSpace());
}

void PSOutputDev::updateRender(GfxState *state) {
  writePS("%d Tr\n", state->getRender());
}

void PSOutputDev::updateRise(GfxState *state) {
  writePS("%g Ts\n", state->getRise());
}

void PSOutputDev::updateWordSpace(GfxState *state) {
  writePS("%g Tw\n", state->getWordSpace());
}

void PSOutputDev::updateHorizScaling(GfxState *state) {
  writePS("%g Tz\n", state->getHorizScaling());
}

void PSOutputDev::updateTextPos(GfxState *state) {
  writePS("%g %g Td\n", state->getLineX(), state->getLineY());
}

void PSOutputDev::updateTextShift(GfxState *state, double shift) {
  writePS("%g TJm\n", shift);
}

void PSOutputDev::stroke(GfxState *state) {
  doPath(state->getPath());
  writePS("S\n");
}

void PSOutputDev::fill(GfxState *state) {
  doPath(state->getPath());
  writePS("f\n");
}

void PSOutputDev::eoFill(GfxState *state) {
  doPath(state->getPath());
  writePS("f*\n");
}

void PSOutputDev::clip(GfxState *state) {
  doPath(state->getPath());
  writePS("W\n");
}

void PSOutputDev::eoClip(GfxState *state) {
  doPath(state->getPath());
  writePS("W*\n");
}

void PSOutputDev::doPath(GfxPath *path) {
  GfxSubpath *subpath;
  int n, m, i, j;

  n = path->getNumSubpaths();
  for (i = 0; i < n; ++i) {
    subpath = path->getSubpath(i);
    m = subpath->getNumPoints();
    writePS("%g %g m\n", subpath->getX(0), subpath->getY(0));
    j = 1;
    while (j < m) {
      if (subpath->getCurve(j)) {
	writePS("%g %g %g %g %g %g c\n", subpath->getX(j), subpath->getY(j),
		subpath->getX(j+1), subpath->getY(j+1),
		subpath->getX(j+2), subpath->getY(j+2));
	j += 3;
      } else {
	writePS("%g %g l\n", subpath->getX(j), subpath->getY(j));
	++j;
      }
    }
  }
}

void PSOutputDev::drawString(GfxState *state, GString *s) {
  writePSString(s);
  writePS(" %g Tj\n", state->getFont()->getWidth(s));
}

void PSOutputDev::drawImageMask(GfxState *state, Stream *str,
				int width, int height, GBool invert,
				GBool inlineImg) {
  doImage(gTrue, inlineImg, str, width, height);
}

void PSOutputDev::drawImage(GfxState *state, Stream *str, int width,
			    int height, GfxColorSpace *colorSpace,
			    GBool inlineImg) {
  doImage(gFalse, inlineImg, str, width, height);
}

void PSOutputDev::doImage(GBool mask, GBool inlineImg, Stream *str,
			  int width, int height) {
  Dict *dict;
  GBool indexed;
  Object obj1, obj2;
  char *s;
  GString *s1;
  int numComps, bits;
  int c;
  int n, i;

  // get image dictionary
  dict = str->getDict();

  // color space
  numComps = 1;
  indexed = gFalse;
  if (!mask) {
    if (dict->lookup("ColorSpace", &obj1)->isNull()) {
      obj1.free();
      dict->lookup("CS", &obj1);
    }
    numComps = 1;
    if (obj1.isName("DeviceGray") || obj1.isName("G")) {
      writePS("/DeviceGray setcolorspace\n");
      numComps = 1;
    } else if (obj1.isName("DeviceRGB") || obj1.isName("RGB")) {
      writePS("/DeviceRGB setcolorspace\n");
      numComps = 3;
    } else if (obj1.isName("DeviceCMYK") || obj1.isName("CMYK")) {
      writePS("/DeviceCMYK setcolorspace\n");
      numComps = 4;
    } else if (obj1.isArray()) {
      obj1.arrayGet(0, &obj2);
      if (obj2.isName("DeviceGray") || obj2.isName("G")) {
	writePS("/DeviceGray setcolorspace\n");
	numComps = 1;
      } else if (obj2.isName("DeviceRGB") || obj2.isName("RGB")) {
	writePS("/DeviceRGB setcolorspace\n");
	numComps = 3;
      } else if (obj2.isName("DeviceCMYK") || obj2.isName("CMYK")) {
	writePS("/DeviceCMYK setcolorspace\n");
	numComps = 4;
      } else if (obj2.isName("Indexed") || obj2.isName("I")) {
	indexed = gTrue;
	numComps = 1;
	obj2.free();
	obj1.arrayGet(1, &obj2);
	n = 1;
	if (obj2.isName("DeviceGray") || obj2.isName("G")) {
	  writePS("[/Indexed /DeviceGray");
	  n = 1;
	} else if (obj2.isName("DeviceRGB") || obj2.isName("RGB")) {
	  writePS("[/Indexed /DeviceRGB");
	  n = 3;
	} else if (obj2.isName("DeviceCMYK") || obj2.isName("CMYK")) {
	  writePS("[/Indexed /DeviceCMYK");
	  n = 4;
	}
	obj2.free();
	if (obj1.arrayGet(2, &obj2)->isInt()) {
	  writePS(" %d\n", obj2.getInt());
	  n *= (1 + obj2.getInt());
	}
	obj2.free();
	obj1.arrayGet(3, &obj2);
	writePS("<");
	if (obj2.isStream()) {
	  obj2.streamReset();
	  for (i = 0; i < n; ++i) {
	    if ((c = obj2.streamGetChar()) == EOF)
	      writePS("00");
	    else
	      writePS("%02x", c);
	  }
	} else if (obj2.isString()) {
	  s = obj2.getString()->getCString();
	  for (i = 0; i < n; ++i)
	    writePS("%02x", *s++ & 0xff);
	}
	writePS(">\n] setcolorspace\n");
      }
      obj2.free();
    }
    obj1.free();
  }

  // image dictionary
  writePS("<<\n  /ImageType 1\n");

  // width, height, matrix, bits per component
  writePS("  /Width %d\n", width);
  writePS("  /Height %d\n", height);
  writePS("  /ImageMatrix [%d 0 0 %d 0 %d]\n", width, -height, height);
  if (dict->lookup("BitsPerComponent", &obj1)->isNull()) {
    obj1.free();
    dict->lookup("BPC", &obj1);
  }
  bits = 0;
  if (obj1.isInt())
    writePS("  /BitsPerComponent %d\n", bits = obj1.getInt());
  obj1.free();

  // decode 
  if (dict->lookup("Decode", &obj1)->isNull()) {
    obj1.free();
    dict->lookup("D", &obj1);
  }
  writePS("  /Decode [");
  if (obj1.isArray()) {
    n = obj1.arrayGetLength();
    for (i = 0; i < n; ++i) {
      if (obj1.arrayGet(i, &obj2)->isNum())
	writePS("%g%s", obj2.getNum(), i < n-1 ? " " : "");
      obj2.free();
    }
  } else {
    if (indexed) {
      writePS("0 %d", (1 << bits) - 1);
    } else {
      for (i = 0; i < numComps; ++i)
	writePS("0 1%s", i < numComps-1 ? " " : "");
    }
  }
  writePS("]\n");
  obj1.free();

  // data source
  writePS("  /DataSource currentfile\n");
  if (str->isBinary())
    writePS("    /ASCII85Decode filter\n");
  s1 = str->getPSFilter("    ");
  writePS("%s", s1->getCString());
  delete s1;

  // end of image dictionary
  writePS(">>\n%s\n", mask ? "pdfImM" : "pdfIm");

  // image data
  writeStream(str->getBaseStream(), inlineImg, str->isBinary());
  writePS("%%-EOD-\n");
}

void PSOutputDev::writePS(char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  vfprintf(f, fmt, args);
  va_end(args);
}

void PSOutputDev::writePSString(GString *s) {
  Guchar *p;
  int n;

  fputc('(', f);
  for (p = (Guchar *)s->getCString(), n = s->getLength(); n; ++p, --n) {
    if (*p == '(' || *p == ')' || *p == '\\')
      fprintf(f, "\\%c", *p);
    else if (*p < 0x20 || *p >= 0x80)
      fprintf(f, "\\%03o", *p);
    else
      fputc(*p, f);
  }
  fputc(')', f);
}

void PSOutputDev::writeStream(Stream *str, GBool inlineImg, GBool needA85) {
  int c1, c2, c3;
  int a85[5];
  Gulong t;
  int n, m, i;

  // back to start of stream
  str->reset();

  // for inline image: need to read three chars ahead to check for
  // 'EI' tag
  if (inlineImg) {
    c1 = str->getChar();
    c2 = str->getChar();
  } else {
    c1 = c2 = '\0';
  }

  // do ASCII85 re-encoded stream
  if (needA85) {
    m = 0;
    do {
      t = 0;
      for (n = 0; n < 4; ++n) {
	if (inlineImg) {
	  c3 = str->getChar();
	  if ((c1 == '\n' || c1 == '\r') && c2 == 'E' && c3 == 'I')
	    break;
	  t = (t << 8) + c1;
	  c1 = c2;
	  c2 = c3;
	} else {
	  if ((c1 = str->getChar()) == EOF)
	    break;
	  t = (t << 8) + c1;
	}
      }
      if (n > 0) {
	if (n == 4 && t == 0) {
	  fputc('z', f);
	  if (++m == 65) {
	    fputc('\n', f);
	    m = 0;
	  }
	} else {
	  if (n < 4)
	    t <<= 8 * (4 - n);
	  for (i = 4; i >= 0; --i) {
	    a85[i] = t % 85;
	    t /= 85;
	  }
	  for (i = 0; i <= n; ++i) {
	    fputc((char)(a85[i] + 0x21), f);
	    if (++m == 65) {
	      fputc('\n', f);
	      m = 0;
	    }
	  }
	}
      }
    } while (n == 4);
    fputc('~', f);
    fputc('>', f);

  // stream is already ASCII, just copy it
  } else {
    while (1) {
      if (inlineImg) {
	c3 = str->getChar();
	if ((c1 == '\n' || c1 == '\r') && c2 == 'E' && c3 == 'I')
	  break;
      } else {
	if ((c1 = str->getChar()) == EOF)
	  break;
      }
      fputc(c1, f);
    }
  }

  // add a newline to the end
  fputc('\n', f);
}
