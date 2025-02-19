//========================================================================
//
// Gfx.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef GFX_H
#define GFX_H

#pragma interface

#include <stypes.h>

class Array;
class Stream;
class Parser;
class Dict;
class OutputDev;
class GfxFontDict;
class GfxState;
struct Operator;

//------------------------------------------------------------------------
// Gfx
//------------------------------------------------------------------------

enum TchkType {
  tchkBool,
  tchkInt,
  tchkNum,
  tchkString,
  tchkName,
  tchkArray
};

enum GfxClipType {
  clipNone,
  clipNormal,
  clipEO
};

class Gfx {
public:

  // Constructor.
  Gfx(OutputDev *out1, Dict *fontDict, Dict *xObjDict1,
      int dpi, int x1, int y1, int x2, int y2, int rotate);

  // Destructor.
  ~Gfx();

  // Interpret an array or stream.
  void display(Array *a1);
  void display(Stream *str1);

private:

  OutputDev *out;		// output device
  GfxFontDict *fonts;		// font dictionary
  Dict *xObjDict;		// XObject dictionary

  GfxState *state;		// current graphics state
  Boolean fontChanged;		// set if font or text matrix has changed
  GfxClipType clip;		// do a clip?

  Array *a;			// if displaying from an array
  int index;			// index in array
  Parser *parser;		// if displaying from a stream

  static Operator opTab[];	// table of operators

  void go();
  void execOp(Object *cmd, Object args[], int numArgs);
  Operator *findOp(char *name);
  Boolean checkArg(Object *arg, TchkType type);
  Object *nextObj(Object *obj);
  void done();

  // graphics state operators
  void opSave(Object args[]);
  void opRestore(Object args[]);
  void opConcat(Object args[]);
  void opSetDash(Object args[]);
  void opSetFlat(Object args[]);
  void opSetLineJoin(Object args[]);
  void opSetLineCap(Object args[]);
  void opSetMiterLimit(Object args[]);
  void opSetLineWidth(Object args[]);

  // color operators
  void opSetFillGray(Object args[]);
  void opSetStrokeGray(Object args[]);
  void opSetFillCMYKColor(Object args[]);
  void opSetStrokeCMYKColor(Object args[]);
  void opSetFillRGBColor(Object args[]);
  void opSetStrokeRGBColor(Object args[]);

  // path segment operators
  void opMoveTo(Object args[]);
  void opLineTo(Object args[]);
  void opCurveTo(Object args[]);
  void opCurveTo1(Object args[]);
  void opCurveTo2(Object args[]);
  void opRectangle(Object args[]);
  void opClosePath(Object args[]);

  // path painting operators
  void opEndPath(Object args[]);
  void opStroke(Object args[]);
  void opCloseStroke(Object args[]);
  void opFill(Object args[]);
  void opEOFill(Object args[]);
  void opFillStroke(Object args[]);
  void opCloseFillStroke(Object args[]);
  void opEOFillStroke(Object args[]);
  void opCloseEOFillStroke(Object args[]);
  void doEndPath();

  // path clipping operators
  void opClip(Object args[]);
  void opEOClip(Object args[]);

  // text object operators
  void opBeginText(Object args[]);
  void opEndText(Object args[]);

  // text state operators
  void opSetCharSpacing(Object args[]);
  void opSetFont(Object args[]);
  void opSetTextLeading(Object args[]);
  void opSetTextRender(Object args[]);
  void opSetTextRise(Object args[]);
  void opSetWordSpacing(Object args[]);
  void opSetHorizScaling(Object args[]);

  // text positioning operators
  void opTextMove(Object args[]);
  void opTextMoveSet(Object args[]);
  void opSetTextMatrix(Object args[]);
  void opTextNextLine(Object args[]);

  // text string operators
  void opShowText(Object args[]);
  void opMoveShowText(Object args[]);
  void opMoveSetShowText(Object args[]);
  void opShowSpaceText(Object args[]);
  void doShowText(uchar *s);

  // XObject operators
  void opXObject(Object args[]);
  void doImage(Stream *str);
  void doForm(Stream *str);

  // in-line image operators
  void opBeginImage(Object args[]);
  void opEndImage(Object args[]);
  void opImageData(Object args[]);

  // type 3 font operators
  void opSetCharWidth(Object args[]);
  void opSetCacheDevice(Object args[]);
};

#endif
