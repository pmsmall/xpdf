//========================================================================
//
// PSOutputDev.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef PSOUTPUTDEV_H
#define PSOUTPUTDEV_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include "config.h"
#include "Object.h"
#include "OutputDev.h"

class GfxPath;
class GfxFont;

//------------------------------------------------------------------------
// PSOutputDev
//------------------------------------------------------------------------

enum PSFileType {
  psFile,			// write to file
  psPipe,			// write to pipe
  psStdout			// write to stdout
};

class PSOutputDev: public OutputDev {
public:

  // Open a PostScript output file, and write the prolog.
  PSOutputDev(char *fileName, Catalog *catalog,
	      int firstPage, int lastPage, GBool embedType11);

  // Destructor -- writes the trailer and closes the file.
  virtual ~PSOutputDev();

  // Check if file was successfully created.
  virtual GBool isOk() { return ok; }

  //---- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() { return gFalse; }

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar() { return gFalse; }

  //----- initialization and control

  // Start a page.
  virtual void startPage(int pageNum, GfxState *state);

  // End a page.
  virtual void endPage();

  //----- save/restore graphics state
  virtual void saveState(GfxState *state);
  virtual void restoreState(GfxState *state);

  //----- update graphics state
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
  virtual void updateTextMat(GfxState *state);
  virtual void updateCharSpace(GfxState *state);
  virtual void updateRender(GfxState *state);
  virtual void updateRise(GfxState *state);
  virtual void updateWordSpace(GfxState *state);
  virtual void updateHorizScaling(GfxState *state);
  virtual void updateTextPos(GfxState *state);
  virtual void updateTextShift(GfxState *state, double shift);

  //----- path painting
  virtual void stroke(GfxState *state);
  virtual void fill(GfxState *state);
  virtual void eoFill(GfxState *state);

  //----- path clipping
  virtual void clip(GfxState *state);
  virtual void eoClip(GfxState *state);

  //----- text drawing
  virtual void drawString(GfxState *state, GString *s);

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Stream *str,
			     int width, int height, GBool invert,
			     GBool inlineImg);
  virtual void drawImage(GfxState *state, Stream *str, int width,
			 int height, GfxColorSpace *colorSpace,
			 GBool inlineImg);

private:

  void setupFont(GfxFont *font);
  void setupEmbeddedFont(Ref *id);
  void doPath(GfxPath *path);
  void doImage(GBool mask, GBool inlineImg, Stream *str,
	       int width, int height, int len);
  void writePS(char *fmt, ...);
  void writePSString(GString *s);

  GBool embedType1;		// embed Type 1 fonts?

  FILE *f;			// PostScript file
  PSFileType fileType;		// file / pipe / stdout
  int seqPage;			// current sequential page number

  Ref *fontIDs;			// list of object IDs of all used fonts
  int fontIDLen;		// number of entries in fontIDs array
  int fontIDSize;		// size of fontIDs array
  Ref *fontFileIDs;		// list of object IDs of all embedded fonts
  int fontFileIDLen;		// number of entries in fontFileIDs array
  int fontFileIDSize;		// size of fontFileIDs array

  GBool ok;			// set up ok?
};

#endif
