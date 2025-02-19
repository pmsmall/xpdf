//========================================================================
//
// Gfx.cc
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
#include <gmem.h>
#include <cover.h>
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "Stream.h"
#include "Lexer.h"
#include "Parser.h"
#include "GfxFont.h"
#include "GfxState.h"
#include "OutputDev.h"
#include "PSOutput.h"
#include "Flags.h"
#include "Error.h"
#include "Gfx.h"

//------------------------------------------------------------------------
// Operator table
//------------------------------------------------------------------------

Operator Gfx::opTab[] = {
  {"\"", 3, {tchkNum,    tchkNum,    tchkString},
         &Gfx::opMoveSetShowText,    &Gfx::psMoveSetShowText},
  {"'",  1, {tchkString},
         &Gfx::opMoveShowText,       &Gfx::psMoveShowText},
  {"B",  0, {tchkNone},
         &Gfx::opFillStroke,         &Gfx::psFillStroke},
  {"B*", 0, {tchkNone},
         &Gfx::opEOFillStroke,       &Gfx::psEOFillStroke},
  {"BI", 0, {tchkNone},
         &Gfx::opBeginImage,         &Gfx::psBeginImage},
  {"BT", 0, {tchkNone},
         &Gfx::opBeginText,          &Gfx::psBeginText},
  {"Do", 1, {tchkName},
         &Gfx::opXObject,            &Gfx::psXObject},
  {"EI", 0, {tchkNone},
         &Gfx::opEndImage,           &Gfx::psEndImage},
  {"ET", 0, {tchkNone},
         &Gfx::opEndText,            &Gfx::psEndText},
  {"F",  0, {tchkNone},
         &Gfx::opFill,               &Gfx::psFill},
  {"G",  1, {tchkNum},
         &Gfx::opSetStrokeGray,      &Gfx::psSetStrokeGray},
  {"ID", 0, {tchkNone},
         &Gfx::opImageData,          &Gfx::psImageData},
  {"J",  1, {tchkInt},
         &Gfx::opSetLineCap,         &Gfx::psSetLineCap},
  {"K",  4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
         &Gfx::opSetStrokeCMYKColor, &Gfx::psSetStrokeCMYKColor},
  {"M",  1, {tchkNum},
         &Gfx::opSetMiterLimit,      &Gfx::psSetMiterLimit},
  {"Q",  0, {tchkNone},
         &Gfx::opRestore,            &Gfx::psRestore},
  {"RG", 3, {tchkNum,    tchkNum,    tchkNum},
         &Gfx::opSetStrokeRGBColor,  &Gfx::psSetStrokeRGBColor},
  {"S",  0, {tchkNone},
         &Gfx::opStroke,             &Gfx::psStroke},
  {"T*", 0, {tchkNone},
         &Gfx::opTextNextLine,       &Gfx::psTextNextLine},
  {"TD", 2, {tchkNum,    tchkNum},
         &Gfx::opTextMoveSet,        &Gfx::psTextMoveSet},
  {"TJ", 1, {tchkArray},
         &Gfx::opShowSpaceText,      &Gfx::psShowSpaceText},
  {"TL", 1, {tchkNum},
         &Gfx::opSetTextLeading,     &Gfx::psSetTextLeading},
  {"Tc", 1, {tchkNum},
         &Gfx::opSetCharSpacing,     &Gfx::psSetCharSpacing},
  {"Td", 2, {tchkNum,    tchkNum},
         &Gfx::opTextMove,           &Gfx::psTextMove},
  {"Tf", 2, {tchkName,   tchkNum},
         &Gfx::opSetFont,            &Gfx::psSetFont},
  {"Tj", 1, {tchkString},
         &Gfx::opShowText,           &Gfx::psShowText},
  {"Tm", 6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	     tchkNum,    tchkNum},
         &Gfx::opSetTextMatrix,      &Gfx::psSetTextMatrix},
  {"Tr", 1, {tchkInt},
         &Gfx::opSetTextRender,      &Gfx::psSetTextRender},
  {"Ts", 1, {tchkNum},
         &Gfx::opSetTextRise,        &Gfx::psSetTextRise},
  {"Tw", 1, {tchkNum},
         &Gfx::opSetWordSpacing,     &Gfx::psSetWordSpacing},
  {"Tz", 1, {tchkNum},
         &Gfx::opSetHorizScaling,    &Gfx::psSetHorizScaling},
  {"W",  0, {tchkNone},
         &Gfx::opClip,               &Gfx::psClip},
  {"W*", 0, {tchkNone},
         &Gfx::opEOClip,             &Gfx::psEOClip},
  {"b",  0, {tchkNone},
         &Gfx::opCloseFillStroke,    &Gfx::psCloseFillStroke},
  {"b*", 0, {tchkNone},
         &Gfx::opCloseEOFillStroke,  &Gfx::psCloseEOFillStroke},
  {"c",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	     tchkNum,    tchkNum},
         &Gfx::opCurveTo,            &Gfx::psCurveTo},
  {"cm", 6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	     tchkNum,    tchkNum},
         &Gfx::opConcat,             &Gfx::psConcat},
  {"d",  2, {tchkArray,  tchkNum},
         &Gfx::opSetDash,            &Gfx::psSetDash},
  {"d0", 2, {tchkNum,    tchkNum},
         &Gfx::opSetCharWidth,       &Gfx::psSetCharWidth},
  {"d1", 2, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	     tchkNum,    tchkNum},
         &Gfx::opSetCacheDevice,     &Gfx::psSetCacheDevice},
  {"f",  0, {tchkNone},
         &Gfx::opFill,               &Gfx::psFill},
  {"f*", 0, {tchkNone},
         &Gfx::opEOFill,             &Gfx::psEOFill},
  {"g",  1, {tchkNum},
         &Gfx::opSetFillGray,        &Gfx::psSetFillGray},
  {"h",  0, {tchkNone},
         &Gfx::opClosePath,          &Gfx::psClosePath},
  {"i",  1, {tchkNum},
         &Gfx::opSetFlat,            &Gfx::psSetFlat},
  {"j",  1, {tchkInt},
         &Gfx::opSetLineJoin,        &Gfx::psSetLineJoin},
  {"k",  4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
         &Gfx::opSetFillCMYKColor,   &Gfx::psSetFillCMYKColor},
  {"l",  2, {tchkNum,    tchkNum},
         &Gfx::opLineTo,             &Gfx::psLineTo},
  {"m",  2, {tchkNum,    tchkNum},
         &Gfx::opMoveTo,             &Gfx::psMoveTo},
  {"n",  0, {tchkNone},
         &Gfx::opEndPath,            &Gfx::psEndPath},
  {"q",  0, {tchkNone},
         &Gfx::opSave,               &Gfx::psSave},
  {"re", 4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
         &Gfx::opRectangle,          &Gfx::psRectangle},
  {"rg", 3, {tchkNum,    tchkNum,    tchkNum},
         &Gfx::opSetFillRGBColor,    &Gfx::psSetFillRGBColor},
  {"s",  0, {tchkNone},
         &Gfx::opCloseStroke,        &Gfx::psCloseStroke},
  {"v",  4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
         &Gfx::opCurveTo1,           &Gfx::psCurveTo1},
  {"w",  1, {tchkNum},
         &Gfx::opSetLineWidth,       &Gfx::psSetLineWidth},
  {"y",  4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
         &Gfx::opCurveTo2,           &Gfx::psCurveTo2}
};

#define numOps (sizeof(opTab) / sizeof(Operator))

//------------------------------------------------------------------------
// Gfx
//------------------------------------------------------------------------

Gfx::Gfx(OutputDev *out1, Dict *fontDict, Dict *xObjDict1,
	 int dpi, int x1, int y1, int x2, int y2, int rotate) {
  out = out1;
  psOut = NULL;
  if (fontDict)
    fonts = new GfxFontDict(fontDict);
  else
    fonts = NULL;
  xObjDict = xObjDict1;
  state = new GfxState(dpi, x1, y1, x2, y2, rotate, out->upsideDown());
  fontChanged = gFalse;
  clip = clipNone;
  psFont = NULL;
  psFontSize = 1;
  psGfxFont = NULL;
  out->updateAll(state);
  out->setPageSize(state->getPageWidth(), state->getPageHeight());
  out->setCTM(state->getCTM());
  out->clear();
}

Gfx::Gfx(PSOutput *psOut1, Dict *fontDict, Dict *xObjDict1,
	 int dpi, int x1, int y1, int x2, int y2, int rotate) {
  psOut = psOut1;
  out = NULL;
  //~ the fonts are needed to adjust for imperfect match with PS fonts
  if (fontDict)
    fonts = new GfxFontDict(fontDict);
  else
    fonts = NULL;
  xObjDict = xObjDict1;
  state = NULL;
  fontChanged = gFalse;
  clip = clipNone;
  psFont = NULL;
  psFontSize = 1;
  psGfxFont = NULL;
}

Gfx::~Gfx() {
  if (fonts)
    delete fonts;
  if (state)
    delete state;
  if (psFont)
    delete psFont;
}

void Gfx::display(Array *a1) {
  cover("Gfx::display(Array)");
  parser = NULL;
  a = a1;
  index = 0;
  go();
}

void Gfx::display(Stream *str1) {
  cover("Gfx::display(Stream)");
  parser = new Parser(new Lexer(str1, gFalse));
  a = NULL;
  go();
}

void Gfx::go() {
  Object obj;
  Object args[maxArgs];
  int i, j;

  i = 0;
  nextObj(&obj);
  while (!obj.isEOF()) {
    if (obj.isCmd()) {
      if (printCommands) {
	obj.print(stdout);
	for (j = 0; j < i; ++j) {
	  printf(" ");
	  args[j].print(stdout);
	}
	printf("\n");
      }
      execOp(&obj, args, i);
      obj.free();
      for (j = 0; j < i; ++j)
	args[j].free();
      i = 0;
    } else if (i < maxArgs) {
      args[i++] = obj;
    } else {
      error(0, "Too many args in content stream");
      if (printCommands) {
	printf("throwing away arg: ");
	obj.print(stdout);
	printf("\n");
      }
      obj.free();
    }
    nextObj(&obj);
  }
  obj.free();
  if (i > 0) {
    error(0, "Leftover args in content stream");
    for (j = 0; j < i; ++j)
      args[j].free();
  }
  done();
  if (out)
    out->dump();
  if (printCommands)
    fflush(stdout);
}

void Gfx::execOp(Object *cmd, Object args[], int numArgs) {
  Operator *op;
  char *name;
  int i;

  // find operator
  name = cmd->getName();
  if (!(op = findOp(name))) {
    error(0, "Unknown operator '%s'", name);
    return;
  }

  // type check args
  if (numArgs != op->numArgs) {
    error(0, "Wrong number (%d) of args to '%s' operator", numArgs, name);
    return;
  }
  for (i = 0; i < numArgs; ++i) {
    if (!checkArg(&args[i], op->tchk[i])) {
      error(0, "Arg #%d to '%s' operator is wrong type (%s)",
	    i, name, args[i].getTypeName());
      return;
    }
  }

  // do it
  if (out)
    (this->*op->func)(args);
  else
    (this->*op->psFunc)(args);
}

Operator *Gfx::findOp(char *name) {
  int a, b, m, cmp;

  a = -1;
  b = numOps;
  // invariant: opTab[a] < name < opTab[b]
  while (b - a > 1) {
    m = (a + b) / 2;
    cmp = strcmp(opTab[m].name, name);
    if (cmp < 0)
      a = m;
    else if (cmp > 0)
      b = m;
    else
      a = b = m;
  }
  if (cmp != 0)
    return NULL;
  return &opTab[a];
}

GBool Gfx::checkArg(Object *arg, TchkType type) {
  switch (type) {
  case tchkBool:   return arg->isBool();
  case tchkInt:    return arg->isInt();
  case tchkNum:    return arg->isNum();
  case tchkString: return arg->isString();
  case tchkName:   return arg->isName();
  case tchkArray:  return arg->isArray();
  case tchkNone:   return gFalse;
  }
  return gFalse;
}

Object *Gfx::nextObj(Object *obj) {
  if (a) {
    if (index < a->getLength())
      a->get(index++, obj);
    else
      obj->initEOF();
  } else {
    parser->getObj(obj);
  }
  return obj;
}

void Gfx::done() {
  if (parser)
    delete parser;
}

//------------------------------------------------------------------------
// graphics state operators
//------------------------------------------------------------------------

void Gfx::opSave(Object args[]) {
  cover("Gfx::opSave");
  out->saveState(state);
  state = state->save();
}

void Gfx::psSave(Object args[]) {
  psOut->writePS("q\n");
}

void Gfx::opRestore(Object args[]) {
  cover("Gfx::opRestore");
  state = state->restore();
  out->restoreState(state);
}

void Gfx::psRestore(Object args[]) {
  psOut->writePS("Q\n");
}

void Gfx::opConcat(Object args[]) {
  cover("Gfx::opConcat");
  state->concatCTM(args[0].getNum(), args[1].getNum(),
		   args[2].getNum(), args[3].getNum(),
		   args[4].getNum(), args[5].getNum());
  out->updateCTM(state);
  fontChanged = gTrue;
}

void Gfx::psConcat(Object args[]) {
  psOut->writePS("[%g %g %g %g %g %g] cm\n",
		 args[0].getNum(), args[1].getNum(),
		 args[2].getNum(), args[3].getNum(),
		 args[4].getNum(), args[5].getNum());
}

void Gfx::opSetDash(Object args[]) {
  Array *a;
  int length;
  Object obj;
  double *dash;
  int i;

  cover("Gfx::opSetDash");
  a = args[0].getArray();
  length = a->getLength();
  if (length == 0) {
    dash = NULL;
  } else {
    dash = (double *)gmalloc(length * sizeof(double));
    for (i = 0; i < length; ++i) {
      dash[i] = a->get(i, &obj)->getNum();
      obj.free();
    }
  }
  state->setLineDash(dash, length, args[1].getNum());
  out->updateLineDash(state);
}

void Gfx::psSetDash(Object args[]) {
  Array *a;
  int length;
  Object obj;
  int i;

  a = args[0].getArray();
  length = a->getLength();
  psOut->writePS("[");
  for (i = 0; i < length; ++i) {
    psOut->writePS("%g%s", a->get(i, &obj)->getNum(),
		   (i == length-1) ? "" : " ");
    obj.free();
  }
  psOut->writePS("] %g d\n", args[1].getNum());
}

void Gfx::opSetFlat(Object args[]) {
  cover("Gfx::opSetFlat");
  state->setFlatness((int)args[0].getNum());
  out->updateFlatness(state);
}

void Gfx::psSetFlat(Object args[]) {
  psOut->writePS("%g i\n", args[0].getNum());
}

void Gfx::opSetLineJoin(Object args[]) {
  cover("Gfx::opSetLineJoin");
  state->setLineJoin(args[0].getInt());
  out->updateLineJoin(state);
}

void Gfx::psSetLineJoin(Object args[]) {
  psOut->writePS("%d j\n", args[0].getInt());
}

void Gfx::opSetLineCap(Object args[]) {
  cover("Gfx::opSetLineCap");
  state->setLineCap(args[0].getInt());
  out->updateLineCap(state);
}

void Gfx::psSetLineCap(Object args[]) {
  psOut->writePS("%d J\n", args[0].getInt());
}

void Gfx::opSetMiterLimit(Object args[]) {
  cover("Gfx::opSetMiterLimit");
  state->setMiterLimit(args[0].getNum());
  out->updateMiterLimit(state);
}

void Gfx::psSetMiterLimit(Object args[]) {
  psOut->writePS("%g M\n", args[0].getNum());
}

void Gfx::opSetLineWidth(Object args[]) {
  cover("Gfx::opSetLineWidth");
  state->setLineWidth(args[0].getNum());
  out->updateLineWidth(state);
}

void Gfx::psSetLineWidth(Object args[]) {
  psOut->writePS("%g w\n", args[0].getNum());
}

//------------------------------------------------------------------------
// color operators
//------------------------------------------------------------------------

void Gfx::opSetFillGray(Object args[]) {
  cover("Gfx::opSetFillGray");
  state->setFillGray(args[0].getNum());
  out->updateFillColor(state);
}

void Gfx::psSetFillGray(Object args[]) {
  psOut->writePS("%g g\n", args[0].getNum());
}

void Gfx::opSetStrokeGray(Object args[]) {
  cover("Gfx::opSetStrokeGray");
  state->setStrokeGray(args[0].getNum());
  out->updateStrokeColor(state);
}

void Gfx::psSetStrokeGray(Object args[]) {
  psOut->writePS("%g G\n", args[0].getNum());
}

void Gfx::opSetFillCMYKColor(Object args[]) {
  cover("Gfx::opSetFillCMYKColor");
  state->setFillCMYK(args[0].getNum(), args[1].getNum(),
		     args[2].getNum(), args[3].getNum());
  out->updateFillColor(state);
}

void Gfx::psSetFillCMYKColor(Object args[]) {
  psOut->writePS("%g %g %g %g k\n",
		 args[0].getNum(), args[1].getNum(),
		 args[2].getNum(), args[3].getNum());
}

void Gfx::opSetStrokeCMYKColor(Object args[]) {
  cover("Gfx::opSetStrokeCMYKColor");
  state->setStrokeCMYK(args[0].getNum(), args[1].getNum(),
		       args[2].getNum(), args[3].getNum());
  out->updateStrokeColor(state);
}

void Gfx::psSetStrokeCMYKColor(Object args[]) {
  psOut->writePS("%g %g %g %g K\n",
		 args[0].getNum(), args[1].getNum(),
		 args[2].getNum(), args[3].getNum());
}

void Gfx::opSetFillRGBColor(Object args[]) {
  cover("Gfx::opSetFillRGBColor");
  state->setFillRGB(args[0].getNum(), args[1].getNum(), args[2].getNum());
  out->updateFillColor(state);
}

void Gfx::psSetFillRGBColor(Object args[]) {
  psOut->writePS("%g %g %g rg\n",
		 args[0].getNum(), args[1].getNum(), args[2].getNum());
}

void Gfx::opSetStrokeRGBColor(Object args[]) {
  cover("Gfx::opSetStrokeRGBColor");
  state->setStrokeRGB(args[0].getNum(), args[1].getNum(), args[2].getNum());
  out->updateStrokeColor(state);
}

void Gfx::psSetStrokeRGBColor(Object args[]) {
  psOut->writePS("%g %g %g RG\n",
		 args[0].getNum(), args[1].getNum(), args[2].getNum());
}

//------------------------------------------------------------------------
// path segment operators
//------------------------------------------------------------------------

void Gfx::opMoveTo(Object args[]) {
  cover("Gfx::opMoveTo");
  state->moveTo(args[0].getNum(), args[1].getNum());
}

void Gfx::psMoveTo(Object args[]) {
  psOut->writePS("%g %g m\n", args[0].getNum(), args[1].getNum());
}

void Gfx::opLineTo(Object args[]) {
  cover("Gfx::opLineTo");
  if (!state->isPath()) {
    error(0, "No current point in lineto");
    return;
  }
  state->lineTo(args[0].getNum(), args[1].getNum());
}

void Gfx::psLineTo(Object args[]) {
  psOut->writePS("%g %g l\n", args[0].getNum(), args[1].getNum());
}

void Gfx::opCurveTo(Object args[]) {
  double x1, y1, x2, y2, x3, y3;

  cover("Gfx::opCurveTo");
  if (!state->isPath()) {
    error(0, "No current point in curveto");
    return;
  }
  x1 = args[0].getNum();
  y1 = args[1].getNum();
  x2 = args[2].getNum();
  y2 = args[3].getNum();
  x3 = args[4].getNum();
  y3 = args[5].getNum();
  state->curveTo(x1, y1, x2, y2, x3, y3);
}

void Gfx::psCurveTo(Object args[]) {
  psOut->writePS("%g %g %g %g %g %g c\n",
		 args[0].getNum(), args[1].getNum(), args[2].getNum(),
		 args[3].getNum(), args[4].getNum(), args[5].getNum());
}

void Gfx::opCurveTo1(Object args[]) {
  double x1, y1, x2, y2, x3, y3;

  cover("Gfx::opCurveTo1");
  if (!state->isPath()) {
    error(0, "No current point in curveto1");
    return;
  }
  x1 = state->getCurX();
  y1 = state->getCurY();
  x2 = args[0].getNum();
  y2 = args[1].getNum();
  x3 = args[2].getNum();
  y3 = args[3].getNum();
  state->curveTo(x1, y1, x2, y2, x3, y3);
}

void Gfx::psCurveTo1(Object args[]) {
  psOut->writePS("%g %g %g %g v\n",
		 args[0].getNum(), args[1].getNum(),
		 args[2].getNum(), args[3].getNum());
}

void Gfx::opCurveTo2(Object args[]) {
  double x1, y1, x2, y2, x3, y3;

  cover("Gfx::opCurveTo2");
  if (!state->isPath()) {
    error(0, "No current point in curveto2");
    return;
  }
  x1 = args[0].getNum();
  y1 = args[1].getNum();
  x2 = args[2].getNum();
  y2 = args[3].getNum();
  x3 = x2;
  y3 = y2;
  state->curveTo(x1, y1, x2, y2, x3, y3);
}

void Gfx::psCurveTo2(Object args[]) {
  psOut->writePS("%g %g %g %g y\n",
		 args[0].getNum(), args[1].getNum(),
		 args[2].getNum(), args[3].getNum());
}

void Gfx::opRectangle(Object args[]) {
  double x, y, w, h;

  cover("Gfx::opRectangle");
  x = args[0].getNum();
  y = args[1].getNum();
  w = args[2].getNum();
  h = args[3].getNum();
  state->moveTo(x, y);
  state->lineTo(x + w, y);
  state->lineTo(x + w, y + h);
  state->lineTo(x, y + h);
  state->closePath();
}

void Gfx::psRectangle(Object args[]) {
  psOut->writePS("%g %g %g %g re\n",
		 args[0].getNum(), args[1].getNum(),
		 args[2].getNum(), args[3].getNum());
}

void Gfx::opClosePath(Object args[]) {
  GfxPath *path;

  cover("Gfx::opClosePath");
  if (!state->isPath()) {
    error(0, "No current point in closepath");
    return;
  }
  state->closePath();
  path = state->getPath();
}

void Gfx::psClosePath(Object args[]) {
  psOut->writePS("h\n");
}

//------------------------------------------------------------------------
// path painting operators
//------------------------------------------------------------------------

void Gfx::opEndPath(Object args[]) {
  cover("Gfx::opEndPath");
  doEndPath();
}

void Gfx::psEndPath(Object args[]) {
  if (clip != clipNone)
    psDoClip(gFalse);
  psOut->writePS("n\n");
}

void Gfx::opStroke(Object args[]) {
  cover("Gfx::opStroke");
  if (!state->isPath()) {
    error(0, "No path in stroke");
    return;
  }
  out->stroke(state);
  doEndPath();
}

void Gfx::psStroke(Object args[]) {
  if (clip != clipNone)
    psOut->writePS("gsave\n");
  psOut->writePS("S\n");
  if (clip != clipNone)
    psDoClip(gTrue);
}

void Gfx::opCloseStroke(Object args[]) {
  cover("Gfx::opCloseStroke");
  if (!state->isPath()) {
    error(0, "No path in closepath/stroke");
    return;
  }
  state->closePath();
  out->stroke(state);
  doEndPath();
}

void Gfx::psCloseStroke(Object args[]) {
  if (clip != clipNone)
    psOut->writePS("gsave\n");
  psOut->writePS("s\n");
  if (clip != clipNone)
    psDoClip(gTrue);
}

void Gfx::opFill(Object args[]) {
  cover("Gfx::opFill");
  if (!state->isPath()) {
    error(0, "No path in fill");
    return;
  }
  out->fill(state);
  doEndPath();
}

void Gfx::psFill(Object args[]) {
  if (clip != clipNone)
    psOut->writePS("gsave\n");
  psOut->writePS("f\n");
  if (clip != clipNone)
    psDoClip(gTrue);
}

void Gfx::opEOFill(Object args[]) {
  cover("Gfx::opEOFill");
  if (!state->isPath()) {
    error(0, "No path in eofill");
    return;
  }
  out->eoFill(state);
  doEndPath();
}

void Gfx::psEOFill(Object args[]) {
  if (clip != clipNone)
    psOut->writePS("gsave\n");
  psOut->writePS("f*\n");
  if (clip != clipNone)
    psDoClip(gTrue);
}

void Gfx::opFillStroke(Object args[]) {
  cover("Gfx::opFillStroke");
  if (!state->isPath()) {
    error(0, "No path in fill/stroke");
    return;
  }
  out->fill(state);
  out->stroke(state);
  doEndPath();
}

void Gfx::psFillStroke(Object args[]) {
  if (clip != clipNone)
    psOut->writePS("gsave\n");
  psOut->writePS("B\n");
  if (clip != clipNone)
    psDoClip(gTrue);
}

void Gfx::opCloseFillStroke(Object args[]) {
  cover("Gfx::opCloseFillStroke");
  if (!state->isPath()) {
    error(0, "No path in closepath/fill/stroke");
    return;
  }
  state->closePath();
  out->fill(state);
  out->stroke(state);
  doEndPath();
}

void Gfx::psCloseFillStroke(Object args[]) {
  if (clip != clipNone)
    psOut->writePS("gsave\n");
  psOut->writePS("b\n");
  if (clip != clipNone)
    psDoClip(gTrue);
}

void Gfx::opEOFillStroke(Object args[]) {
  cover("Gfx::opEOFillStroke");
  if (!state->isPath()) {
    error(0, "No path in eofill/stroke");
    return;
  }
  out->eoFill(state);
  out->stroke(state);
  doEndPath();
}

void Gfx::psEOFillStroke(Object args[]) {
  if (clip != clipNone)
    psOut->writePS("gsave\n");
  psOut->writePS("B*\n");
  if (clip != clipNone)
    psDoClip(gTrue);
}

void Gfx::opCloseEOFillStroke(Object args[]) {
  cover("Gfx::opCloseEOFillStroke");
  if (!state->isPath()) {
    error(0, "No path in closepath/eofill/stroke");
    return;
  }
  state->closePath();
  out->eoFill(state);
  out->stroke(state);
  doEndPath();
}

void Gfx::psCloseEOFillStroke(Object args[]) {
  if (clip != clipNone)
    psOut->writePS("gsave\n");
  psOut->writePS("b*\n");
  if (clip != clipNone)
    psDoClip(gTrue);
}

void Gfx::doEndPath() {
  if (clip == clipNormal)
    out->clip(state);
  else if (clip == clipEO)
    out->eoClip(state);
  clip = clipNone;
  state->clearPath();
}

void Gfx::psDoClip(GBool needRestore) {
  if (needRestore)
    psOut->writePS("grestore\n");
  if (clip == clipNormal)
    psOut->writePS("clip\n");
  else
    psOut->writePS("eoclip\n");
  if (needRestore)
    psOut->writePS("newpath\n");
  clip = clipNone;
}

//------------------------------------------------------------------------
// path clipping operators
//------------------------------------------------------------------------

void Gfx::opClip(Object args[]) {
  cover("Gfx::opClip");
  clip = clipNormal;
}

void Gfx::psClip(Object args[]) {
  clip = clipNormal;
}

void Gfx::opEOClip(Object args[]) {
  cover("Gfx::opEOClip");
  clip = clipEO;
}

void Gfx::psEOClip(Object args[]) {
  clip = clipEO;
}

//------------------------------------------------------------------------
// text object operators
//------------------------------------------------------------------------

void Gfx::opBeginText(Object args[]) {
  cover("Gfx::opBeginText");
  state->setTextMat(1, 0, 0, 1, 0, 0);
  state->textMoveTo(0, 0);
  fontChanged = gTrue;
}

void Gfx::psBeginText(Object args[]) {
  psOut->writePS("[1 0 0 1 0 0] Tm\n");
  fontChanged = gTrue;
}

void Gfx::opEndText(Object args[]) {
  cover("Gfx::opEndText");
}

void Gfx::psEndText(Object args[]) {
}

//------------------------------------------------------------------------
// text state operators
//------------------------------------------------------------------------

void Gfx::opSetCharSpacing(Object args[]) {
  cover("Gfx::opSetCharSpacing");
  state->setCharSpace(args[0].getNum());
}

void Gfx::psSetCharSpacing(Object args[]) {
  psOut->writePS("%g Tc\n", args[0].getNum());
}

void Gfx::opSetFont(Object args[]) {
  GfxFont *font;

  cover("Gfx::opSetFont");
  if (!fonts) {
    error(0, "setfont without font dictionary");
    return;
  }
  if (!(font = fonts->lookup(args[0].getName()))) {
    error(0, "unknown font tag '%s'", args[0].getName());
    return;
  }
  if (printCommands) {
    printf("  font: '%s' %g\n",
	   font->getName() ? font->getName()->getCString() : "???",
	   args[1].getNum());
  }
  state->setFont(font, args[1].getNum());
  fontChanged = gTrue;
}

void Gfx::psSetFont(Object args[]) {
  if (fonts)
    psGfxFont = fonts->lookup(args[0].getName());
  if (psFont)
    delete psFont;
  psFont = new GString(args[0].getName());
  psFontSize = args[1].getNum();
  fontChanged = gTrue;
}

void Gfx::opSetTextLeading(Object args[]) {
  cover("Gfx::opSetTextLeading");
  state->setLeading(args[0].getNum());
}

void Gfx::psSetTextLeading(Object args[]) {
  psOut->writePS("%g TL\n", args[0].getNum());
}

void Gfx::opSetTextRender(Object args[]) {
  cover("Gfx::opSetTextRender");
  state->setRender(args[0].getInt());
}

void Gfx::psSetTextRender(Object args[]) {
  int r;

  r = args[0].getInt();
  psOut->writePS("%d Tr\n", r);
}

void Gfx::opSetTextRise(Object args[]) {
  cover("Gfx::opSetTextRise");
  state->setRise(args[0].getNum());
}

void Gfx::psSetTextRise(Object args[]) {
  psOut->writePS("%g Ts\n", args[0].getNum());
}

void Gfx::opSetWordSpacing(Object args[]) {
  cover("Gfx::opSetWordSpacing");
  state->setWordSpace(args[0].getNum());
}

void Gfx::psSetWordSpacing(Object args[]) {
  psOut->writePS("%g Tw\n", args[0].getNum());
}

void Gfx::opSetHorizScaling(Object args[]) {
  cover("Gfx::opSetHorizScaling");
  state->setHorizScaling(args[0].getNum());
}

void Gfx::psSetHorizScaling(Object args[]) {
  psOut->writePS("%g Tz\n", args[0].getNum());
}

//------------------------------------------------------------------------
// text positioning operators
//------------------------------------------------------------------------

void Gfx::opTextMove(Object args[]) {
  double tx, ty;

  cover("Gfx::opTextMove");
  tx = state->getLineX() + args[0].getNum();
  ty = state->getLineY() + args[1].getNum();
  state->textMoveTo(tx, ty);
}

void Gfx::psTextMove(Object args[]) {
  psOut->writePS("%g %g Td\n", args[0].getNum(), args[1].getNum());
}

void Gfx::opTextMoveSet(Object args[]) {
  double tx, ty;

  cover("Gfx::opTextMoveSet");
  tx = state->getLineX() + args[0].getNum();
  ty = args[1].getNum();
  state->setLeading(-ty);
  ty += state->getLineY();
  state->textMoveTo(tx, ty);
}

void Gfx::psTextMoveSet(Object args[]) {
  psOut->writePS("%g %g TD\n", args[0].getNum(), args[1].getNum());
}

void Gfx::opSetTextMatrix(Object args[]) {
  cover("Gfx::opSetTextMatrix");
  state->setTextMat(args[0].getNum(), args[1].getNum(),
		    args[2].getNum(), args[3].getNum(),
		    args[4].getNum(), args[5].getNum());
  state->textMoveTo(0, 0);
  fontChanged = gTrue;
}

void Gfx::psSetTextMatrix(Object args[]) {
  psOut->writePS("[%g %g %g %g %g %g] Tm\n",
		 args[0].getNum(), args[1].getNum(),
		 args[2].getNum(), args[3].getNum(),
		 args[4].getNum(), args[5].getNum());
  fontChanged = gTrue;
}

void Gfx::opTextNextLine(Object args[]) {
  double tx, ty;

  cover("Gfx::opTextNextLine");
  tx = state->getLineX();
  ty = state->getLineY() - state->getLeading();
  state->textMoveTo(tx, ty);
}

void Gfx::psTextNextLine(Object args[]) {
  psOut->writePS("T*\n");
}

//------------------------------------------------------------------------
// text string operators
//------------------------------------------------------------------------

void Gfx::opShowText(Object args[]) {
  cover("Gfx::opShowText");
  if (!state->getFont()) {
    error(0, "No font in show");
    return;
  }
  doShowText(args[0].getString());
}

void Gfx::psShowText(Object args[]) {
  if (fontChanged) {
    psOut->writePS("/%s %g Tf\n", psFont->getCString(), psFontSize);
    fontChanged = gFalse;
  }
  psOut->writePSString(args[0].getString());
  psOut->writePS(" %g Tj\n", psGfxFont->getWidth(args[0].getString()));
}

void Gfx::opMoveShowText(Object args[]) {
  double tx, ty;

  cover("Gfx::opMoveShowText");
  if (!state->getFont()) {
    error(0, "No font in move/show");
    return;
  }
  tx = state->getLineX();
  ty = state->getLineY() - state->getLeading();
  state->textMoveTo(tx, ty);
  doShowText(args[0].getString());
}

void Gfx::psMoveShowText(Object args[]) {
  if (fontChanged) {
    psOut->writePS("/%s %g Tf\n", psFont->getCString(), psFontSize);
    fontChanged = gFalse;
  }
  psOut->writePS("T* ");
  psOut->writePSString(args[0].getString());
  psOut->writePS(" %g Tj\n", psGfxFont->getWidth(args[0].getString()));
}

void Gfx::opMoveSetShowText(Object args[]) {
  double tx, ty;

  cover("Gfx::opMoveSetShowText");
  if (!state->getFont()) {
    error(0, "No font in move/set/show");
    return;
  }
  state->setWordSpace(args[0].getNum());
  state->setCharSpace(args[1].getNum());
  tx = state->getLineX();
  ty = state->getLineY() - state->getLeading();
  state->textMoveTo(tx, ty);
  doShowText(args[2].getString());
}

void Gfx::psMoveSetShowText(Object args[]) {
  if (fontChanged) {
    psOut->writePS("/%s %g Tf\n", psFont->getCString(), psFontSize);
    fontChanged = gFalse;
  }
  psOut->writePS("%g Tw %g Tc T* ", args[0].getNum(), args[1].getNum());
  psOut->writePSString(args[2].getString());
  psOut->writePS(" %g Tj\n", psGfxFont->getWidth(args[2].getString()));
}

void Gfx::opShowSpaceText(Object args[]) {
  Array *a;
  Object obj;
  int i;

  cover("Gfx::opShowSpaceText");
  if (!state->getFont()) {
    error(0, "No font in show/space");
    return;
  }
  a = args[0].getArray();
  for (i = 0; i < a->getLength(); ++i) {
    a->get(i, &obj);
    if (obj.isNum())
      state->textShift(-obj.getNum() * 0.001 * state->getFontSize());
    else if (obj.isString())
      doShowText(obj.getString());
    else
      error(0, "Element of show/space array must be number or string");
    obj.free();
  }
}

void Gfx::psShowSpaceText(Object args[]) {
  Array *a;
  Object obj;
  int i;

  if (fontChanged) {
    psOut->writePS("/%s %g Tf\n", psFont->getCString(), psFontSize);
    fontChanged = gFalse;
  }
  a = args[0].getArray();
  for (i = 0; i < a->getLength(); ++i) {
    a->get(i, &obj);
    if (obj.isNum()) {
      psOut->writePS("%g TJm\n", obj.getNum());
    } else if (obj.isString()) {
      psOut->writePSString(obj.getString());
      psOut->writePS(" %g Tj\n", psGfxFont->getWidth(obj.getString()));
    }
    obj.free();
  }
}

void Gfx::doShowText(GString *s) {
  Guchar *p;
  int n;
  double dx, dy;

  if (fontChanged) {
    out->updateFont(state);
    fontChanged = gFalse;
  }
  state->textTransformDelta(0, state->getRise(), &dx, &dy);
  for (p = (Guchar *)s->getCString(), n = s->getLength(); n; ++p, --n) {
    if (*p != '\r' && *p != '\n') {
      out->drawChar(state, state->getCurX() + dx, state->getCurY() + dy, *p);
      if (*p == ' ')
	state->textShift(state->getWordSpace());
    }
    state->textShift(state->getFontSize() * state->getFont()->getWidth(*p) +
		     state->getCharSpace());
  }
}

//------------------------------------------------------------------------
// XObject operators
//------------------------------------------------------------------------

void Gfx::opXObject(Object args[]) {
  Object obj1, obj2;

  cover("Gfx::opXObject");
  if (!xObjDict) {
    error(0, "No XObject dictionary in 'Do' operator");
    return;
  }
  xObjDict->lookup(args[0].getName(), &obj1);
  if (!obj1.isStream("XObject")) {
    error(0, "XObject '%s' is unknown or wrong type", args[0].getName());
    obj1.free();
    return;
  }
  obj1.streamGetDict()->lookup("Subtype", &obj2);
  if (obj2.isName("Image"))
    doImage(obj1.getStream());
  else if (obj2.isName("Form"))
    doForm(obj1.getStream());
  else if (obj2.isName())
    error(0, "Unknown XObject subtype '%s'", obj2.getName());
  else
    error(0, "XObject subtype is missing or wrong type");
  obj2.free();
  obj1.free();
}

void Gfx::psXObject(Object args[]) {
  Object obj1, obj2;

  if (!xObjDict) {
    error(0, "No XObject dictionary in 'Do' operator");
    return;
  }
  xObjDict->lookup(args[0].getName(), &obj1);
  if (!obj1.isStream("XObject")) {
    error(0, "XObject '%s' is unknown or wrong type", args[0].getName());
    obj1.free();
    return;
  }
  obj1.streamGetDict()->lookup("Subtype", &obj2);
  if (obj2.isName("Image"))
    psDoImage(obj1.getStream());
  else if (obj2.isName("Form"))
    psDoForm(obj1.getStream());
  else if (obj2.isName())
    error(0, "Unknown XObject subtype '%s'", obj2.getName());
  else
    error(0, "XObject subtype is missing or wrong type");
  obj2.free();
  obj1.free();
}

void Gfx::doImage(Stream *str) {
  Dict *dict;
  Object obj1, obj2;
  int width, height;
  int bits;
  GBool mask;
  GfxColorSpace *colorSpace;
  GBool invert;

  cover("Gfx::doImage");

  // get stream dict
  dict = str->getDict();

  // get size
  dict->lookup("Width", &obj1);
  if (obj1.isNull()) {
    obj1.free();
    dict->lookup("W", &obj1);
  }
  if (!obj1.isInt())
    goto err2;
  width = obj1.getInt();
  obj1.free();
  dict->lookup("Height", &obj1);
  if (obj1.isNull()) {
    obj1.free();
    dict->lookup("H", &obj1);
  }
  if (!obj1.isInt())
    goto err2;
  height = obj1.getInt();
  obj1.free();

  // image or mask?
  dict->lookup("ImageMask", &obj1);
  if (obj1.isNull()) {
    obj1.free();
    dict->lookup("IM", &obj1);
  }
  mask = gFalse;
  if (obj1.isBool())
    mask = obj1.getBool();
  else if (!obj1.isNull())
    goto err2;
  obj1.free();

  // bit depth
  dict->lookup("BitsPerComponent", &obj1);
  if (obj1.isNull()) {
    obj1.free();
    dict->lookup("BPC", &obj1);
  }
  if (!obj1.isInt())
    goto err2;
  bits = obj1.getInt();
  obj1.free();

  // display a mask
  if (mask) {
    if (bits != 1)
      goto err1;
    invert = gFalse;
    dict->lookup("Decode", &obj1);
    if (obj1.isNull()) {
      obj1.free();
      dict->lookup("D", &obj1);
    }
    if (obj1.isArray()) {
      obj1.arrayGet(0, &obj2);
      if (obj2.isInt() && obj2.getInt() == 1)
	invert = gTrue;
      obj2.free();
    } else if (!obj1.isNull()) {
      goto err2;
    }
    obj1.free();
    out->drawImageMask(state, str, width, height, invert);

  } else {
    // get color space
    dict->lookup("ColorSpace", &obj1);
    if (obj1.isNull()) {
      obj1.free();
      dict->lookup("CS", &obj1);
    }
    if (!(obj1.isName() || obj1.isArray() || obj1.isNull()))
      goto err2;
    dict->lookup("Decode", &obj2);
    if (obj1.isNull()) {
      obj1.free();
      dict->lookup("D", &obj1);
    }
    if (!(obj2.isArray() || obj2.isNull()))
      goto err3;
    colorSpace = new GfxColorSpace(bits, &obj1, &obj2);
    if (!colorSpace->isOk())
      goto err4;
    obj1.free();
    obj2.free();

    // display an image
    out->drawImage(state, str, width, height, colorSpace);
    delete colorSpace;
  }
  
  return;

 err4:
  delete colorSpace;
 err3:
  obj2.free();
 err2:
  obj1.free();
 err1:
  error(0, "Bad image parameters");
}

void Gfx::psDoImage(Stream *str) {
  psOut->writeImage(str->getDict(), str, gFalse);
}

void Gfx::doForm(Stream *str) {
  cover("Gfx::doForm");
  error(0, "Unimplemented: form");
}

void Gfx::psDoForm(Stream *str) {
  error(0, "Unimplemented: form");
}

//------------------------------------------------------------------------
// in-line image operators
//------------------------------------------------------------------------

void Gfx::opBeginImage(Object args[]) {
  Object obj;
  Stream *str;

  cover("Gfx::opBeginImage");

  // build dict/stream
  str = buildImageStream();

  // display the image
  if (str) {
    doImage(str);
    delete str;
  }

  // get 'EI' tag
  nextObj(&obj);
  if (!obj.isCmd("EI"))
    error(0, "Expected 'EI' operator");
  obj.free();
}

void Gfx::psBeginImage(Object args[]) {
  Stream *str;

  // build dict/stream
  str = buildImageStream();

  // display the image
  // (writeImage() eats the 'EI' tag)
  if (str) {
    psOut->writeImage(str->getDict(), str, gTrue);
    delete str;
  }
}

Stream *Gfx::buildImageStream() {
  Object dict;
  Object obj;
  char *key;
  Stream *str;

  // inline image must be in stream
  if (!parser) {
    error(0, "Inline image in array content stream");
    return NULL;
  }

  // build dictionary
  dict.initDict();
  nextObj(&obj);
  while (!obj.isCmd("ID") && !obj.isEOF()) {
    if (!obj.isName()) {
      error(0, "Dictionary key must be a name object");
      obj.free();
      nextObj(&obj);
    } else {
      key = copyString(obj.getName());
      obj.free();
      nextObj(&obj);
      if (obj.isEOF() || obj.isError())
	break;
      dict.dictAdd(key, &obj);
    }
    nextObj(&obj);
  }
  if (obj.isEOF())
    error(0, "End of file in inline image");
  obj.free();

  // make stream
  str = new SubStream(parser->getStream(), &dict);
  str = str->addFilters(&dict);

  return str;
}

void Gfx::opImageData(Object args[]) {
  error(0, "Internal: got 'ID' operator");
}

void Gfx::psImageData(Object args[]) {
  error(0, "Internal: got 'ID' operator");
}

void Gfx::opEndImage(Object args[]) {
  error(0, "Internal: got 'EI'operator");
}

void Gfx::psEndImage(Object args[]) {
  error(0, "Internal: got 'EI'operator");
}

//------------------------------------------------------------------------
// type 3 font operators
//------------------------------------------------------------------------

void Gfx::opSetCharWidth(Object args[]) {
  cover("Gfx::opSetCharWidth");
  error(0, "Encountered 'd0' operator in content stream");
}

void Gfx::psSetCharWidth(Object args[]) {
}

void Gfx::opSetCacheDevice(Object args[]) {
  cover("Gfx::opSetCacheDevice");
  error(0, "Encountered 'd1' operator in content stream");
}

void Gfx::psSetCacheDevice(Object args[]) {
}
