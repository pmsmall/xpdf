//========================================================================
//
// XOutputDev.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef XOUTPUTDEV_H
#define XOUTPUTDEV_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "config.h"
#include "OutputDev.h"

class GString;
struct GfxRGB;
class GfxFont;
class GfxSubpath;
class TextPage;
class FontEncoding;
class XOutputFontCache;
class Link;
class Catalog;

#if HAVE_T1LIB_H
class T1FontEngine;
class T1FontFile;
class T1Font;
#endif

#if FREETYPE2 && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H)
class FTFontEngine;
class FTFontFile;
class FTFont;
#endif

#if !FREETYPE2 && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H)
class TTFontEngine;
class TTFontFile;
class TTFont;
#endif

//------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------

#define maxRGBCube 8		// max size of RGB color cube

#define numTmpPoints 256	// number of XPoints in temporary array
#define numTmpSubpaths 16	// number of elements in temporary arrays
				//   for fill/clip

//------------------------------------------------------------------------
// Misc types
//------------------------------------------------------------------------

// Font rasterizer control.
enum FontRastControl {
  fontRastNone,			// don't use this rasterizer
  fontRastPlain,		// use it, without anti-aliasing
  fontRastAALow,		// use it, with low-level anti-aliasing
  fontRastAAHigh		// use it, with high-level anti-aliasing
};

struct BoundingRect {
  short xMin, xMax;		// min/max x values
  short yMin, yMax;		// min/max y values
};

//------------------------------------------------------------------------
// Parameters
//------------------------------------------------------------------------

// If any of these are set, xpdf will use t1lib to render those font(s)
// instead of using the X server font(s).
extern GString *t1Courier;
extern GString *t1CourierBold;
extern GString *t1CourierBoldOblique;
extern GString *t1CourierOblique;
extern GString *t1Helvetica;
extern GString *t1HelveticaBold;
extern GString *t1HelveticaBoldOblique;
extern GString *t1HelveticaOblique;
extern GString *t1Symbol;
extern GString *t1TimesBold;
extern GString *t1TimesBoldItalic;
extern GString *t1TimesItalic;
extern GString *t1TimesRoman;
extern GString *t1ZapfDingbats;

// Use the EUC-JP encoding.
extern GBool useEUCJP;

#if JAPANESE_SUPPORT
// X font name pattern to use for Japanese text.
extern GString *japan12Font;
#endif

#if CHINESE_GB_SUPPORT
// X font name pattern to use for Chinese GB text.
extern GString *gb12Font;
#endif

#if CHINESE_CNS_SUPPORT
// X font name pattern to use for Chinese CNS text.
extern GString *cns13Font;
#endif

//------------------------------------------------------------------------
// XOutputFont
//------------------------------------------------------------------------

class XOutputFont {
public:

  XOutputFont(GfxFont *gfxFont, double m11, double m12,
	      double m21, double m22, Display *displayA,
	      XOutputFontCache *cache);

  virtual ~XOutputFont();

  // Does this font match the ID and transform?
  GBool matches(Ref id1, double m11, double m12, double m21, double m22)
    { return id.num == id1.num && id.gen == id1.gen &&
	     m11 == tm11 && m12 == tm12 && m21 == tm21 && m22 == tm22; }

  // Was font created successfully?
  virtual GBool isOk() = 0;

  // Update <gc> with this font.
  virtual void updateGC(GC gc) = 0;

  // Draw character <c> at <x>,<y>.
  virtual void drawChar(GfxState *state, Pixmap pixmap, int w, int h,
			GC gc, double x, double y, int c) = 0;

  // Does this font use hex char codes?
  GBool isHex() { return hex; }

protected:

  Ref id;			// font ID
  double tm11, tm12,		// original transform matrix
         tm21, tm22;
  Display *display;		// X display
  GBool hex;			// subsetted font with hex char codes
				//   (this flag is used for text output)
};

#if HAVE_T1LIB_H
//------------------------------------------------------------------------
// XOutputT1Font
//------------------------------------------------------------------------

class XOutputT1Font: public XOutputFont {
public:

  XOutputT1Font(XRef *xref, GfxFont *gfxFont,
		GString *pdfBaseFont,
		double m11, double m12, double m21, double m22,
		Display *displayA, XOutputFontCache *cache);

  virtual ~XOutputT1Font();

  // Was font created successfully?
  virtual GBool isOk();

  // Update <gc> with this font.
  virtual void updateGC(GC gc);

  // Draw character <c> at <x>,<y>.
  virtual void drawChar(GfxState *state, Pixmap pixmap, int w, int h,
			GC gc, double x, double y, int c);

private:

  T1FontFile *fontFile;
  T1Font *font;
};
#endif // HAVE_T1LIB_H

#if FREETYPE2 && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H)
//------------------------------------------------------------------------
// XOutputFTFont
//------------------------------------------------------------------------

class XOutputFTFont: public XOutputFont {
public:

  XOutputFTFont(XRef *xref, GfxFont *gfxFont,
		GString *pdfBaseFont,
		double m11, double m12, double m21, double m22,
		Display *displayA, XOutputFontCache *cache);

  virtual ~XOutputFTFont();

  // Was font created successfully?
  virtual GBool isOk();

  // Update <gc> with this font.
  virtual void updateGC(GC gc);

  // Draw character <c> at <x>,<y>.
  virtual void drawChar(GfxState *state, Pixmap pixmap, int w, int h,
			GC gc, double x, double y, int c);

private:

  FTFontFile *fontFile;
  FTFont *font;
};
#endif // FREETYPE2 && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H)

#if !FREETYPE2 && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H)
//------------------------------------------------------------------------
// XOutputTTFont
//------------------------------------------------------------------------

class XOutputTTFont: public XOutputFont {
public:

  XOutputTTFont(XRef *xref, GfxFont *gfxFont,
		double m11, double m12, double m21, double m22,
		Display *displayA, XOutputFontCache *cache);

  virtual ~XOutputTTFont();

  // Was font created successfully?
  virtual GBool isOk();

  // Update <gc> with this font.
  virtual void updateGC(GC gc);

  // Draw character <c> at <x>,<y>.
  virtual void drawChar(GfxState *state, Pixmap pixmap, int w, int h,
			GC gc, double x, double y, int c);

private:

  TTFontFile *fontFile;
  TTFont *font;
};
#endif // !FREETYPE2 && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H)

//------------------------------------------------------------------------
// XOutputServerFont
//------------------------------------------------------------------------

class XOutputServerFont: public XOutputFont {
public:

  XOutputServerFont(GfxFont *gfxFont, char *fontNameFmt,
		    FontEncoding *encoding,
		    double m11, double m12, double m21, double m22,
		    double size, double ntm11, double ntm12,
		    double ntm21, double ntm22,
		    Display *displayA, XOutputFontCache *cache);

  virtual ~XOutputServerFont();

  // Was font created successfully?
  virtual GBool isOk();

  // Update <gc> with this font.
  virtual void updateGC(GC gc);

  // Draw character <c> at <x>,<y>.
  virtual void drawChar(GfxState *state, Pixmap pixmap, int w, int h,
			GC gc, double x, double y, int c);

private:

  XFontStruct *xFont;		// the X font
  Gushort map[256];		// forward map (PDF code -> font code)
  Guchar revMap[256];		// reverse map (font code -> PDF code)
};

//------------------------------------------------------------------------
// XOutputFontCache
//------------------------------------------------------------------------

#if HAVE_T1LIB_H
struct XOutputT1FontFile {
  int num, gen;
  T1FontFile *fontFile;
};
#endif

#if FREETYPE2 && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H)
struct XOutputFTFontFile {
  int num, gen;
  FTFontFile *fontFile;
};
#endif

#if !FREETYPE2 && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H)
struct XOutputTTFontFile {
  int num, gen;
  TTFontFile *fontFile;
};
#endif

class XOutputFontCache {
public:

  // Constructor.
  XOutputFontCache(Display *displayA, Guint depthA,
		   FontRastControl t1libControlA,
		   FontRastControl freetypeControlA);

  // Destructor.
  ~XOutputFontCache();

  // Initialize (or re-initialize) the font cache for a new document.
  void startDoc(int screenNum, Colormap colormap,
		GBool trueColor,
		int rMul, int gMul, int bMul,
		int rShift, int gShift, int bShift,
		Gulong *colors, int numColors);

  // Get a font.  This creates a new font if necessary.
  XOutputFont *getFont(XRef *xref, GfxFont *gfxFont, double m11, double m12,
		       double m21, double m22);

#if HAVE_T1LIB_H
  // Get a t1lib font file.
  T1FontFile *getT1Font(XRef *xref, GfxFont *gfxFont,
			GString *pdfBaseFont);
#endif

#if FREETYPE2 && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H)
  // Get a FreeType font file.
  FTFontFile *getFTFont(XRef *xref, GfxFont *gfxFont, GString *pdfBaseFont);
#endif

#if !FREETYPE2 && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H)
  // Get a FreeType font file.
  TTFontFile *getTTFont(XRef *xref, GfxFont *gfxFont);
#endif

private:

  void delFonts();
  void clear();

  Display *display;		// X display pointer
  Guint depth;			// pixmap depth

#if HAVE_T1LIB_H
  FontRastControl t1libControl;	// t1lib settings
  T1FontEngine *t1Engine;	// Type 1 font engine
  XOutputT1Font *		// Type 1 fonts in reverse-LRU order
    t1Fonts[t1FontCacheSize];
  int nT1Fonts;			// number of valid entries in t1Fonts[]
  XOutputT1FontFile *		// list of Type 1 font files
    t1FontFiles;
  int t1FontFilesSize;		// size of t1FontFiles array
#endif

#if HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H
  FontRastControl		// FreeType settings
    freetypeControl;
  GBool useFreeType;		// if false, FreeType is not used at all
  GBool freeTypeAA;		// true for anti-aliased fonts
#endif
#if FREETYPE2 && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H)
  FTFontEngine *ftEngine;	// FreeType font engine
  XOutputFTFont *		// FreeType fonts in reverse-LRU order
    ftFonts[ftFontCacheSize];
  int nFTFonts;			// number of valid entries in ftFonts[]
  XOutputFTFontFile *		// list of FreeType font files
    ftFontFiles;
  int ftFontFilesSize;		// size of ftFontFiles array
#endif
#if !FREETYPE2 && (HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H)
  TTFontEngine *ttEngine;	// TrueType font engine
  XOutputTTFont *		// TrueType fonts in reverse-LRU order
    ttFonts[ttFontCacheSize];
  int nTTFonts;			// number of valid entries in ttFonts[]
  XOutputTTFontFile *		// list of TrueType font files
    ttFontFiles;
  int ttFontFilesSize;		// size of ttFontFiles array
#endif

  XOutputServerFont *		// X server fonts in reverse-LRU order
    serverFonts[serverFontCacheSize];
  int nServerFonts;		// number of valid entries in serverFonts[]
};

//------------------------------------------------------------------------
// XOutputState
//------------------------------------------------------------------------

struct XOutputState {
  GC strokeGC;
  GC fillGC;
  Region clipRegion;
  XOutputState *next;
};

//------------------------------------------------------------------------
// XOutputDev
//------------------------------------------------------------------------

class XOutputDev: public OutputDev {
public:

  // Constructor.
  XOutputDev(Display *displayA, Pixmap pixmapA, Guint depthA,
	     Colormap colormapA, unsigned long paperColor,
	     GBool installCmap, int rgbCubeSize,
	     FontRastControl t1libControl,
	     FontRastControl freeTypeControl);

  // Destructor.
  virtual ~XOutputDev();

  //---- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() { return gTrue; }

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar() { return gTrue; }

  //----- initialization and control

  // Start a page.
  virtual void startPage(int pageNum, GfxState *state);

  // End a page.
  virtual void endPage();

  //----- link borders
  virtual void drawLink(Link *link, Catalog *catalog);

  //----- save/restore graphics state
  virtual void saveState(GfxState *state);
  virtual void restoreState(GfxState *state);

  //----- update graphics state
  virtual void updateAll(GfxState *state);
  virtual void updateCTM(GfxState *state, double m11, double m12,
			 double m21, double m22, double m31, double m32);
  virtual void updateLineDash(GfxState *state);
  virtual void updateFlatness(GfxState *state);
  virtual void updateLineJoin(GfxState *state);
  virtual void updateLineCap(GfxState *state);
  virtual void updateMiterLimit(GfxState *state);
  virtual void updateLineWidth(GfxState *state);
  virtual void updateFillColor(GfxState *state);
  virtual void updateStrokeColor(GfxState *state);

  //----- update text state
  virtual void updateFont(GfxState *state);

  //----- path painting
  virtual void stroke(GfxState *state);
  virtual void fill(GfxState *state);
  virtual void eoFill(GfxState *state);

  //----- path clipping
  virtual void clip(GfxState *state);
  virtual void eoClip(GfxState *state);

  //----- text drawing
  virtual void beginString(GfxState *state, GString *s);
  virtual void endString(GfxState *state);
  virtual void drawChar(GfxState *state, double x, double y,
			double dx, double dy, Guchar c);
  virtual void drawChar16(GfxState *state, double x, double y,
			  double dx, double dy, int c);

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Object *ref, Stream *str,
			     int width, int height, GBool invert,
			     GBool inlineImg);
  virtual void drawImage(GfxState *state, Object *ref, Stream *str,
			 int width, int height, GfxImageColorMap *colorMap,
			 int *maskColors, GBool inlineImg);

  //----- special access

  // Called to indicate that a new PDF document has been loaded.
  void startDoc(XRef *xrefA);

  // Find a string.  If <top> is true, starts looking at <xMin>,<yMin>;
  // otherwise starts looking at top of page.  If <bottom> is true,
  // stops looking at <xMax>,<yMax>; otherwise stops looking at bottom
  // of page.  If found, sets the text bounding rectange and returns
  // true; otherwise returns false.
  GBool findText(char *s, GBool top, GBool bottom,
		 int *xMin, int *yMin, int *xMax, int *yMax);

  // Get the text which is inside the specified rectangle.
  GString *getText(int xMin, int yMin, int xMax, int yMax);

protected:

  // Update pixmap ID after a page change.
  void setPixmap(Pixmap pixmap1, int pixmapW1, int pixmapH1)
    { pixmap = pixmap1; pixmapW = pixmapW1; pixmapH = pixmapH1; }

private:

  XRef *xref;			// the xref table for this PDF file
  Display *display;		// X display pointer
  int screenNum;		// X screen number
  Pixmap pixmap;		// pixmap to draw into
  int pixmapW, pixmapH;		// size of pixmap
  Guint depth;			// pixmap depth
  Colormap colormap;		// X colormap
  int flatness;			// line flatness
  GC paperGC;			// GC for background
  GC strokeGC;			// GC with stroke color
  GC fillGC;			// GC with fill color
  Region clipRegion;		// clipping region
  GBool trueColor;		// set if using a TrueColor visual
  int rMul, gMul, bMul;		// RGB multipliers (for TrueColor)
  int rShift, gShift, bShift;	// RGB shifts (for TrueColor)
  Gulong			// color cube
    colors[maxRGBCube * maxRGBCube * maxRGBCube];
  int numColors;		// size of color cube
  XPoint			// temporary points array
    tmpPoints[numTmpPoints];
  int				// temporary arrays for fill/clip
    tmpLengths[numTmpSubpaths];
  BoundingRect
    tmpRects[numTmpSubpaths];
  GfxFont *gfxFont;		// current PDF font
  XOutputFont *font;		// current font
  XOutputFontCache *fontCache;	// font cache
  XOutputState *save;		// stack of saved states
  TextPage *text;		// text from the current page
  GBool type3Warning;		// only show the Type 3 font warning once

  void updateLineAttrs(GfxState *state, GBool updateDash);
  void doFill(GfxState *state, int rule);
  void doClip(GfxState *state, int rule);
  int convertPath(GfxState *state, XPoint **points, int *size,
		  int *numPoints, int **lengths, GBool fillHack);
  void convertSubpath(GfxState *state, GfxSubpath *subpath,
		      XPoint **points, int *size, int *n);
  void doCurve(XPoint **points, int *size, int *k,
	       double x0, double y0, double x1, double y1,
	       double x2, double y2, double x3, double y3);
  void addPoint(XPoint **points, int *size, int *k, int x, int y);
  Gulong findColor(GfxRGB *rgb);
  Gulong findColor(GfxRGB *x, GfxRGB *err);
};

#endif
