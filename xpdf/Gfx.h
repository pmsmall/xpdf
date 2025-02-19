//========================================================================
//
// Gfx.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef GFX_H
#define GFX_H

#ifdef __GNUC__
#pragma interface
#endif

#include <gtypes.h>

class GString;
class Array;
class Stream;
class Parser;
class Dict;
class OutputDev;
class PSOutput;
class GfxFontDict;
class GfxState;
class Gfx;

//------------------------------------------------------------------------
// Gfx
//------------------------------------------------------------------------

enum TchkType {
  tchkBool,
  tchkInt,
  tchkNum,
  tchkString,
  tchkName,
  tchkArray,
  tchkNone			// used to avoid empty initializer lists
};

enum GfxClipType {
  clipNone,
  clipNormal,
  clipEO
};

#define maxArgs 8

struct Operator {
  char name[4];
  int numArgs;
  TchkType tchk[maxArgs];
  void (Gfx::*func)(Object args[]);
  void (Gfx::*psFunc)(Object args[]);
};

class Gfx {
public:

  // Constructor for regular output.
  Gfx(OutputDev *out1, Dict *fontDict, Dict *xObjDict1,
      int dpi, int x1, int y1, int x2, int y2, int rotate);

  // Constructor for PostScript output.
  Gfx(PSOutput *psOut1, Dict *fontDict, Dict *xObjDict1,
      int dpi, int x1, int y1, int x2, int y2, int rotate);

  // Destructor.
  ~Gfx();

  // Interpret an array or stream.
  void display(Array *a1);
  void display(Stream *str1);

private:

  OutputDev *out;		// output device
  PSOutput *psOut;		// PostScript output file
  GfxFontDict *fonts;		// font dictionary
  Dict *xObjDict;		// XObject dictionary

  GfxState *state;		// current graphics state
  GBool fontChanged;		// set if font or text matrix has changed
  GfxClipType clip;		// do a clip?

  GString *psFont;		// font (for PS output)
  double psFontSize;		// font size (for PS output)
  GfxFont *psGfxFont;		// font info (for PS output)

  Array *a;			// if displaying from an array
  int index;			// index in array
  Parser *parser;		// if displaying from a stream

  static Operator opTab[];	// table of operators

  void go();
  void execOp(Object *cmd, Object args[], int numArgs);
  Operator *findOp(char *name);
  GBool checkArg(Object *arg, TchkType type);
  Object *nextObj(Object *obj);
  void done();

  // graphics state operators
  void opSave(Object args[]);
  void psSave(Object args[]);
  void opRestore(Object args[]);
  void psRestore(Object args[]);
  void opConcat(Object args[]);
  void psConcat(Object args[]);
  void opSetDash(Object args[]);
  void psSetDash(Object args[]);
  void opSetFlat(Object args[]);
  void psSetFlat(Object args[]);
  void opSetLineJoin(Object args[]);
  void psSetLineJoin(Object args[]);
  void opSetLineCap(Object args[]);
  void psSetLineCap(Object args[]);
  void opSetMiterLimit(Object args[]);
  void psSetMiterLimit(Object args[]);
  void opSetLineWidth(Object args[]);
  void psSetLineWidth(Object args[]);

  // color operators
  void opSetFillGray(Object args[]);
  void psSetFillGray(Object args[]);
  void opSetStrokeGray(Object args[]);
  void psSetStrokeGray(Object args[]);
  void opSetFillCMYKColor(Object args[]);
  void psSetFillCMYKColor(Object args[]);
  void opSetStrokeCMYKColor(Object args[]);
  void psSetStrokeCMYKColor(Object args[]);
  void opSetFillRGBColor(Object args[]);
  void psSetFillRGBColor(Object args[]);
  void opSetStrokeRGBColor(Object args[]);
  void psSetStrokeRGBColor(Object args[]);

  // path segment operators
  void opMoveTo(Object args[]);
  void psMoveTo(Object args[]);
  void opLineTo(Object args[]);
  void psLineTo(Object args[]);
  void opCurveTo(Object args[]);
  void psCurveTo(Object args[]);
  void opCurveTo1(Object args[]);
  void psCurveTo1(Object args[]);
  void opCurveTo2(Object args[]);
  void psCurveTo2(Object args[]);
  void opRectangle(Object args[]);
  void psRectangle(Object args[]);
  void opClosePath(Object args[]);
  void psClosePath(Object args[]);

  // path painting operators
  void opEndPath(Object args[]);
  void psEndPath(Object args[]);
  void opStroke(Object args[]);
  void psStroke(Object args[]);
  void opCloseStroke(Object args[]);
  void psCloseStroke(Object args[]);
  void opFill(Object args[]);
  void psFill(Object args[]);
  void opEOFill(Object args[]);
  void psEOFill(Object args[]);
  void opFillStroke(Object args[]);
  void psFillStroke(Object args[]);
  void opCloseFillStroke(Object args[]);
  void psCloseFillStroke(Object args[]);
  void opEOFillStroke(Object args[]);
  void psEOFillStroke(Object args[]);
  void opCloseEOFillStroke(Object args[]);
  void psCloseEOFillStroke(Object args[]);
  void doEndPath();
  void psDoClip(GBool needRestore);

  // path clipping operators
  void opClip(Object args[]);
  void psClip(Object args[]);
  void opEOClip(Object args[]);
  void psEOClip(Object args[]);

  // text object operators
  void opBeginText(Object args[]);
  void psBeginText(Object args[]);
  void opEndText(Object args[]);
  void psEndText(Object args[]);

  // text state operators
  void opSetCharSpacing(Object args[]);
  void psSetCharSpacing(Object args[]);
  void opSetFont(Object args[]);
  void psSetFont(Object args[]);
  void opSetTextLeading(Object args[]);
  void psSetTextLeading(Object args[]);
  void opSetTextRender(Object args[]);
  void psSetTextRender(Object args[]);
  void opSetTextRise(Object args[]);
  void psSetTextRise(Object args[]);
  void opSetWordSpacing(Object args[]);
  void psSetWordSpacing(Object args[]);
  void opSetHorizScaling(Object args[]);
  void psSetHorizScaling(Object args[]);

  // text positioning operators
  void opTextMove(Object args[]);
  void psTextMove(Object args[]);
  void opTextMoveSet(Object args[]);
  void psTextMoveSet(Object args[]);
  void opSetTextMatrix(Object args[]);
  void psSetTextMatrix(Object args[]);
  void opTextNextLine(Object args[]);
  void psTextNextLine(Object args[]);

  // text string operators
  void opShowText(Object args[]);
  void psShowText(Object args[]);
  void opMoveShowText(Object args[]);
  void psMoveShowText(Object args[]);
  void opMoveSetShowText(Object args[]);
  void psMoveSetShowText(Object args[]);
  void opShowSpaceText(Object args[]);
  void psShowSpaceText(Object args[]);
  void doShowText(GString *s);

  // XObject operators
  void opXObject(Object args[]);
  void psXObject(Object args[]);
  void doImage(Stream *str);
  void psDoImage(Stream *str);
  void doForm(Stream *str);
  void psDoForm(Stream *str);

  // in-line image operators
  void opBeginImage(Object args[]);
  void psBeginImage(Object args[]);
  Stream *buildImageStream();
  void opImageData(Object args[]);
  void psImageData(Object args[]);
  void opEndImage(Object args[]);
  void psEndImage(Object args[]);

  // type 3 font operators
  void opSetCharWidth(Object args[]);
  void psSetCharWidth(Object args[]);
  void opSetCacheDevice(Object args[]);
  void psSetCacheDevice(Object args[]);
};

#endif
