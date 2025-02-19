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
#include "Stream.h"
#include "PSOutputDev.h"

//------------------------------------------------------------------------
// Parameters
//------------------------------------------------------------------------

// Generate Level 1 PostScript?
GBool psOutLevel1 = gFalse;

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
  "  /pdfHorizScaling 1 def",
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
  "      dup pdfHorizScaling mul exch matrix scale",
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
  "      pdfFontSize mul pdfHorizScaling mul",
  "      1 index stringwidth pdfTextMat idtransform pop",
  "      sub 1 index length dup 0 ne { div } { pop pop 0 } ifelse",
  "      pdfWordSpacing 0 pdfTextMat dtransform 32",
  "      4 3 roll pdfCharSpacing add 0 pdfTextMat dtransform",
  "      6 5 roll awidthshow",
  "      0 pdfTextRise neg pdfTextMat dtransform rmoveto } def",
  "/TJm { pdfFontSize 0.001 mul mul neg 0",
  "       pdfTextMat dtransform rmoveto } def",
  "% Level 1 image operators",
  "/pdfIm1 {",
  "  /pdfImBuf1 4 index string def",
  "  { currentfile pdfImBuf1 readhexstring pop } image",
  "} def",
  "/pdfImM1 {",
  "  /pdfImBuf1 4 index 7 add 8 div string def",
  "  { currentfile pdfImBuf1 readhexstring pop } imagemask",
  "} def",
  "% Level 2 image operators",
  "/pdfImBuf 100 string def",
  "/pdfIm {",
  "  image",
  "  { currentfile pdfImBuf readline",
  "    not { pop exit } if",
  "    (%-EOD-) eq { exit } if } loop",
  "} def",
  "/pdfImM {",
  "  fCol imagemask",
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
			 int firstPage, int lastPage, GBool embedType11) {
  Object fontDict;
  GfxFontDict *gfxFontDict;
  GfxFont *font;
  char **p;
  int pg, i;

  // initialize
  embedType1 = embedType11;
  fontIDs = NULL;
  fontFileIDs = NULL;
  fontFileNames = NULL;

  // open file or pipe
  ok = gTrue;
  if (!strcmp(fileName, "-")) {
    fileType = psStdout;
    f = stdout;
  } else if (fileName[0] == '|') {
    fileType = psPipe;
#ifdef HAVE_POPEN
    signal(SIGPIPE, (void (*)(int))SIG_IGN);
    if (!(f = popen(fileName + 1, "w"))) {
      error(-1, "Couldn't run print command '%s'", fileName);
      ok = gFalse;
      return;
    }
#else
    error(-1, "Print commands are not supported ('%s')", fileName);
    ok = gFalse;
    return;
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

  // initialize fontIDs, fontFileIDs, and fontFileNames lists
  fontIDSize = 64;
  fontIDLen = 0;
  fontIDs = (Ref *)gmalloc(fontIDSize * sizeof(Ref));
  fontFileIDSize = 64;
  fontFileIDLen = 0;
  fontFileIDs = (Ref *)gmalloc(fontFileIDSize * sizeof(Ref));
  fontFileNameSize = 64;
  fontFileNameLen = 0;
  fontFileNames = (char **)gmalloc(fontFileNameSize * sizeof(char *));

  // write document setup
  writePS("%%%%BeginSetup\n");
  for (pg = firstPage; pg <= lastPage; ++pg) {
    catalog->getPage(pg)->getFontDict(&fontDict);
    if (fontDict.isDict()) {
      gfxFontDict = new GfxFontDict(fontDict.getDict());
      for (i = 0; i < gfxFontDict->getNumFonts(); ++i) {
	font = gfxFontDict->getFont(i);
	setupFont(font);
      }
      delete gfxFontDict;
    }
    fontDict.free();
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
#ifdef HAVE_POPEN
    else if (fileType == psPipe) {
      pclose(f);
      signal(SIGPIPE, (void (*)(int))SIG_DFL);
    }
#endif
  }
  if (fontIDs)
    gfree(fontIDs);
  if (fontFileIDs)
    gfree(fontFileIDs);
  if (fontFileNames)
    gfree(fontFileNames);
}

void PSOutputDev::setupFont(GfxFont *font) {
  Ref fontFileID;
  GString *name;
  char *psName;
  char *charName;
  double scale;
  int i, j;

  // check if font is already set up
  for (i = 0; i < fontIDLen; ++i) {
    if (fontIDs[i].num == font->getID().num &&
	fontIDs[i].gen == font->getID().gen)
      return;
  }

  // add entry to fontIDs list
  if (fontIDLen >= fontIDSize) {
    fontIDSize += 64;
    fontIDs = (Ref *)grealloc(fontIDs, fontIDSize * sizeof(Ref));
  }
  fontIDs[fontIDLen++] = font->getID();

  // check for embedded font
  if (embedType1 && font->getType() == fontType1 &&
      font->getEmbeddedFontID(&fontFileID)) {
    setupEmbeddedFont(&fontFileID);
    psName = font->getEmbeddedFontName();
    scale = 1;

  // check for external font file
  } else if (embedType1 && font->getType() == fontType1 &&
	     font->getExtFontFile()) {
    setupEmbeddedFont(font->getExtFontFile());
    // this assumes that the PS font name matches the PDF font name
    psName = font->getName()->getCString();
    scale = 1;

  // do font substitution
  } else {
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
  }

  // generate PostScript code to set up the font
  writePS("/F%d_%d /%s %g\n",
	  font->getID().num, font->getID().gen, psName, scale);
  for (i = 0; i < 256; i += 8) {
    writePS((i == 0) ? "[ " : "  ");
    for (j = 0; j < 8; ++j) {
      charName = font->getCharName(i+j);
      writePS("/%s", charName ? charName : ".notdef");
    }
    writePS((i == 256-8) ? "]\n" : "\n");
  }
  writePS("pdfMakeFont\n");
}

void PSOutputDev::setupEmbeddedFont(Ref *id) {
  static char hexChar[17] = "0123456789abcdef";
  Object refObj, strObj, obj1, obj2;
  Dict *dict;
  int length1, length2;
  int c;
  int start[4];
  GBool binMode;
  int i;

  // check if font is already embedded
  for (i = 0; i < fontFileIDLen; ++i) {
    if (fontFileIDs[i].num == id->num &&
	fontFileIDs[i].gen == id->gen)
      return;
  }

  // add entry to fontFileIDs list
  if (fontFileIDLen >= fontFileIDSize) {
    fontFileIDSize += 64;
    fontFileIDs = (Ref *)grealloc(fontFileIDs, fontFileIDSize * sizeof(Ref));
  }
  fontFileIDs[fontFileIDLen++] = *id;

  // get the font stream and info
  refObj.initRef(id->num, id->gen);
  refObj.fetch(&strObj);
  refObj.free();
  if (!strObj.isStream()) {
    error(-1, "Embedded font file object is not a stream");
    goto err1;
  }
  if (!(dict = strObj.streamGetDict())) {
    error(-1, "Embedded font stream is missing its dictionary");
    goto err1;
  }
  dict->lookup("Length1", &obj1);
  dict->lookup("Length2", &obj2);
  if (!obj1.isInt() || !obj2.isInt()) {
    error(-1, "Missing length fields in embedded font stream dictionary");
    obj1.free();
    obj2.free();
    goto err1;
  }
  length1 = obj1.getInt();
  length2 = obj2.getInt();
  obj1.free();
  obj2.free();

  // copy ASCII portion of font
  strObj.streamReset();
  for (i = 0; i < length1 && (c = strObj.streamGetChar()) != EOF; ++i)
    fputc(c, f);

  // figure out if encrypted portion is binary or ASCII
  binMode = gFalse;
  for (i = 0; i < 4; ++i) {
    start[i] = strObj.streamGetChar();
    if (start[i] == EOF) {
      error(-1, "Unexpected end of file in embedded font stream");
      goto err1;
    }
    if (!((start[i] >= '0' && start[i] <= '9') ||
	  (start[i] >= 'A' && start[i] <= 'F') ||
	  (start[i] >= 'a' && start[i] <= 'f')))
      binMode = gTrue;
  }

  // convert binary data to ASCII
  if (binMode) {
    for (i = 0; i < 4; ++i) {
      fputc(hexChar[(start[i] >> 4) & 0x0f], f);
      fputc(hexChar[start[i] & 0x0f], f);
    }
    while (i < length2) {
      if ((c = strObj.streamGetChar()) == EOF)
	break;
      fputc(hexChar[(c >> 4) & 0x0f], f);
      fputc(hexChar[c & 0x0f], f);
      if (++i % 32 == 0)
	fputc('\n', f);
    }
    if (i % 32 > 0)
      fputc('\n', f);

  // already in ASCII format -- just copy it
  } else {
    for (i = 0; i < 4; ++i)
      fputc(start[i], f);
    for (i = 4; i < length2; ++i) {
      if ((c = strObj.streamGetChar()) == EOF)
	break;
      fputc(c, f);
    }
  }

  // write padding and "cleartomark"
  for (i = 0; i < 8; ++i)
    writePS("00000000000000000000000000000000"
	    "00000000000000000000000000000000\n");
  writePS("cleartomark\n");

 err1:
  strObj.free();
}

//~ This doesn't handle .pfb files or binary eexec data (which only
//~ happens in pfb files?).
void PSOutputDev::setupEmbeddedFont(char *fileName) {
  FILE *fontFile;
  int c;
  int i;

  // check if font is already embedded
  for (i = 0; i < fontFileNameLen; ++i) {
    if (!strcmp(fontFileNames[i], fileName))
      return;
  }

  // add entry to fontFileNames list
  if (fontFileNameLen >= fontFileNameSize) {
    fontFileNameSize += 64;
    fontFileNames = (char **)grealloc(fontFileNames,
				      fontFileNameSize * sizeof(char *));
  }
  fontFileNames[fontFileNameLen++] = fileName;

  // copy the font file
  if (!(fontFile = fopen(fileName, FOPEN_READ_BIN))) {
    error(-1, "Couldn't open external font file");
    return;
  }
  while ((c = fgetc(fontFile)) != EOF)
    fputc(c, f);
  fclose(fontFile);
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

  color = state->getStrokeColor();
  r = color->getR();
  g = color->getG();
  b = color->getB();
  if (r == g && g == b)
    writePS("%g G\n", r);
  else
    writePS("%g %g %g RG\n", r, g, b);
}

void PSOutputDev::updateFont(GfxState *state) {
  if (state->getFont()) {
    writePS("/F%d_%d %g Tf\n",
	    state->getFont()->getID().num, state->getFont()->getID().gen,
	    state->getFontSize());
  }
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
  // check for invisible text -- this is used by Acrobat Capture
  if ((state->getRender() & 3) == 3)
    return;

  writePSString(s);
  writePS(" %g Tj\n", state->getFont()->getWidth(s));
}

void PSOutputDev::drawImageMask(GfxState *state, Stream *str,
				int width, int height, GBool invert,
				GBool inlineImg) {
  int len;

  len = height * ((width + 7) / 8);
  if (psOutLevel1)
    doImageL1(NULL, invert, inlineImg, str, width, height, len);
  else
    doImage(NULL, invert, inlineImg, str, width, height, len);
}

void PSOutputDev::drawImage(GfxState *state, Stream *str, int width,
			    int height, GfxImageColorMap *colorMap,
			    GBool inlineImg) {
  int len;

  len = height * ((width * colorMap->getNumPixelComps() *
		   colorMap->getBits() + 7) / 8);
  if (psOutLevel1)
    doImageL1(colorMap, gFalse, inlineImg, str, width, height, len);
  else
    doImage(colorMap, gFalse, inlineImg, str, width, height, len);
}

void PSOutputDev::doImageL1(GfxImageColorMap *colorMap,
			    GBool invert, GBool inlineImg,
			    Stream *str, int width, int height, int len) {
  Guchar *pixLine;
  GfxColor color;
  int nComps, nBits, nVals;
  Gulong buf, bitMask;
  int bits;
  int x, y, i, j;

  // width, height, matrix, bits per component
  if (colorMap) {
    writePS("%d %d 8 [%d 0 0 %d 0 %d] pdfIm1\n",
	    width, height,
	    width, -height, height);
  } else {
    writePS("%d %d %s [%d 0 0 %d 0 %d] pdfImM1\n",
	    width, height, invert ? "true" : "fase",
	    width, -height, height);
  }

  // set up data stream
  str->reset();

  // image
  if (colorMap) {

    // set up to process the data stream
    nComps = colorMap->getNumPixelComps();
    nBits = colorMap->getBits();
    nVals = width * nComps;
    pixLine = (Guchar *)gmalloc(((nVals + 7) & ~7) * sizeof(Guchar));

    // process the data stream
    i = 0;
    for (y = 0; y < height; ++y) {

      // read a line
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

      // write the line
      for (x = 0, j = 0; x < width; ++x, j += nComps) {
	colorMap->getColor(&pixLine[j], &color);
	fprintf(f, "%02x", (int)(color.getGray() * 255 + 0.5));
	if (++i == 32) {
	  fputc('\n', f);
	  i = 0;
	}
      }
    }
    if (i != 0)
      fputc('\n', f);
    gfree(pixLine);

  // imagemask
  } else {
    i = 0;
    for (y = 0; y < height; ++y) {
      for (x = 0; x < width; x += 8) {
	fprintf(f, "%02x", str->getChar() & 0xff);
	if (++i == 32) {
	  fputc('\n', f);
	  i = 0;
	}
      }
    }
    if (i != 0)
      fputc('\n', f);
  }
}

void PSOutputDev::doImage(GfxImageColorMap *colorMap,
			  GBool invert, GBool inlineImg,
			  Stream *str, int width, int height, int len) {
  GfxColorSpace *colorSpace;
  GString *s;
  int n, numComps;
  Guchar *color;
  GBool useRLE, useA85;
  int c;
  int i, j, k;

  // color space
  if (colorMap) {
    colorSpace = colorMap->getColorSpace();
    if (colorSpace->isIndexed())
      writePS("[/Indexed ");
    switch (colorSpace->getMode()) {
    case colorGray:
      writePS("/DeviceGray ");
      break;
    case colorCMYK:
      writePS("/DeviceCMYK ");
      break;
    case colorRGB:
      writePS("/DeviceRGB ");
      break;
    }
    if (colorSpace->isIndexed()) {
      n = colorSpace->getIndexHigh();
      numComps = colorSpace->getNumColorComps();
      writePS("%d <\n", n);
      for (i = 0; i <= n; i += 8) {
	writePS("  ");
	for (j = i; j < i+8 && j <= n; ++j) {
	  color = colorSpace->getLookupVal(j);
	  for (k = 0; k < numComps; ++k)
	    writePS("%02x", color[k]);
	}
	writePS("\n");
      }
      writePS("> ] setcolorspace\n");
    } else {
      writePS("setcolorspace\n");
    }
  }

  // image dictionary
  writePS("<<\n  /ImageType 1\n");

  // width, height, matrix, bits per component
  writePS("  /Width %d\n", width);
  writePS("  /Height %d\n", height);
  writePS("  /ImageMatrix [%d 0 0 %d 0 %d]\n", width, -height, height);
  writePS("  /BitsPerComponent %d\n",
	  colorMap ? colorMap->getBits() : 1);

  // decode 
  if (colorMap) {
    writePS("  /Decode [");
    numComps = colorMap->getNumPixelComps();
    for (i = 0; i < numComps; ++i) {
      if (i > 0)
	writePS(" ");
      writePS("%g %g", colorMap->getDecodeLow(i), colorMap->getDecodeHigh(i));
    }
    writePS("]\n");
  } else {
    writePS("  /Decode [%d %d]\n", invert ? 1 : 0, invert ? 0 : 1);
  }

  // data source
  writePS("  /DataSource currentfile\n");
  s = str->getPSFilter("    ");
  if (inlineImg || !s) {
    useRLE = gTrue;
    useA85 = gTrue;
  } else {
    useRLE = gFalse;
    useA85 = str->isBinary();
  }
  if (useA85)
    writePS("    /ASCII85Decode filter\n");
  if (useRLE)
    writePS("    /RunLengthDecode filter\n");
  else
    writePS("%s", s->getCString());
  if (s)
    delete s;

  // end of image dictionary
  writePS(">>\n%s\n", colorMap ? "pdfIm" : "pdfImM");

  // write image data stream

  // cut off inline image streams at appropriate length
  if (inlineImg)
    str = new FixedLengthEncoder(str, len);
  else if (!useRLE)
    str = str->getBaseStream();

  // add RunLengthEncode and ASCII85 encode filters
  if (useRLE)
    str = new RunLengthEncoder(str);
  if (useA85)
    str = new ASCII85Encoder(str);

  // copy the stream data
  str->reset();
  while ((c = str->getChar()) != EOF)
    fputc(c, f);

  // add newline and trailer to the end
  fputc('\n', f);
  fputs("%-EOD-\n", f);

  // delete encoders
  if (useRLE || useA85)
    delete str;
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
