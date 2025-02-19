//========================================================================
//
// Gfx.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#pragma implementation

#include <stdio.h>
#include <string.h>
#include <mem.h>
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
#include "Flags.h"
#include "Error.h"
#include "Gfx.h"

//------------------------------------------------------------------------
// Operator table
//------------------------------------------------------------------------

#define maxArgs 8

struct Operator {
  char name[4];
  int numArgs;
  TchkType tchk[maxArgs];
  void (Gfx::*func)(Object args[]);
};

Operator Gfx::opTab[] = {
  {"\"", 3, {tchkNum,    tchkNum,    tchkString},
                                      &Gfx::opMoveSetShowText},
  {"'",  1, {tchkString},             &Gfx::opMoveShowText},
  {"B",  0, {},                       &Gfx::opFillStroke},
  {"B*", 0, {},                       &Gfx::opEOFillStroke},
  {"BI", 0, {},                       &Gfx::opBeginImage},
  {"BT", 0, {},                       &Gfx::opBeginText},
  {"Do", 1, {tchkName},               &Gfx::opXObject},
  {"EI", 0, {},                       &Gfx::opEndImage},
  {"ET", 0, {},                       &Gfx::opEndText},
  {"F",  0, {},                       &Gfx::opFill},
  {"G",  1, {tchkNum},                &Gfx::opSetStrokeGray},
  {"ID", 0, {},                       &Gfx::opImageData},
  {"J",  1, {tchkInt},                &Gfx::opSetLineCap},
  {"K",  4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
                                      &Gfx::opSetStrokeCMYKColor},
  {"M",  1, {tchkNum},                &Gfx::opSetMiterLimit},
  {"Q",  0, {},                       &Gfx::opRestore},
  {"RG", 3, {tchkNum,    tchkNum,    tchkNum},
                                      &Gfx::opSetStrokeRGBColor},
  {"S",  0, {},                       &Gfx::opStroke},
  {"T*", 0, {},                       &Gfx::opTextNextLine},
  {"TD", 2, {tchkNum,    tchkNum},    &Gfx::opTextMoveSet},
  {"TJ", 1, {tchkArray},              &Gfx::opShowSpaceText},
  {"TL", 1, {tchkNum},                &Gfx::opSetTextLeading},
  {"Tc", 1, {tchkNum},                &Gfx::opSetCharSpacing},
  {"Td", 2, {tchkNum,    tchkNum},    &Gfx::opTextMove},
  {"Tf", 2, {tchkName,   tchkNum},    &Gfx::opSetFont},
  {"Tj", 1, {tchkString},             &Gfx::opShowText},
  {"Tm", 6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	     tchkNum,    tchkNum},    &Gfx::opSetTextMatrix},
  {"Tr", 1, {tchkInt},                &Gfx::opSetTextRender},
  {"Ts", 1, {tchkNum},                &Gfx::opSetTextRise},
  {"Tw", 1, {tchkNum},                &Gfx::opSetWordSpacing},
  {"Tz", 1, {tchkNum},                &Gfx::opSetHorizScaling},
  {"W",  0, {},                       &Gfx::opClip},
  {"W*", 0, {},                       &Gfx::opEOClip},
  {"b",  0, {},                       &Gfx::opCloseFillStroke},
  {"b*", 0, {},                       &Gfx::opCloseEOFillStroke},
  {"c",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	     tchkNum,    tchkNum},    &Gfx::opCurveTo},
  {"cm", 6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	     tchkNum,    tchkNum},    &Gfx::opConcat},
  {"d",  2, {tchkArray,  tchkNum},    &Gfx::opSetDash},
  {"d0", 2, {tchkNum,    tchkNum},    &Gfx::opSetCharWidth},
  {"d1", 2, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	     tchkNum,    tchkNum},    &Gfx::opSetCacheDevice},
  {"f",  0, {},                       &Gfx::opFill},
  {"f*", 0, {},                       &Gfx::opEOFill},
  {"g",  1, {tchkNum},                &Gfx::opSetFillGray},
  {"h",  0, {},                       &Gfx::opClosePath},
  {"i",  1, {tchkNum},                &Gfx::opSetFlat},
  {"j",  1, {tchkInt},                &Gfx::opSetLineJoin},
  {"k",  4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
                                      &Gfx::opSetFillCMYKColor},
  {"l",  2, {tchkNum,    tchkNum},    &Gfx::opLineTo},
  {"m",  2, {tchkNum,    tchkNum},    &Gfx::opMoveTo},
  {"n",  0, {},                       &Gfx::opEndPath},
  {"q",  0, {},                       &Gfx::opSave},
  {"re", 4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
                                      &Gfx::opRectangle},
  {"rg", 3, {tchkNum,    tchkNum,    tchkNum},
                                      &Gfx::opSetFillRGBColor},
  {"s",  0, {},                       &Gfx::opCloseStroke},
  {"v",  4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
                                      &Gfx::opCurveTo1},
  {"w",  1, {tchkNum},                &Gfx::opSetLineWidth},
  {"y",  4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
                                      &Gfx::opCurveTo2}
};

#define numOps (sizeof(opTab) / sizeof(Operator))

//------------------------------------------------------------------------
// Gfx
//------------------------------------------------------------------------

Gfx::Gfx(OutputDev *out1, Dict *fontDict, Dict *xObjDict1,
	 int dpi, int x1, int y1, int x2, int y2, int rotate) {
  out = out1;
  if (fontDict)
    fonts = new GfxFontDict(fontDict);
  else
    fonts = NULL;
  xObjDict = xObjDict1;
  state = new GfxState(dpi, x1, y1, x2, y2, rotate, out->upsideDown());
  fontChanged = false;
  out->updateAll(state);
  out->setPageSize(state->getPageWidth(), state->getPageHeight());
  out->clear();
}

Gfx::~Gfx() {
  if (fonts)
    delete fonts;
  delete state;
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
  parser = new Parser(new Lexer(str1, false));
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
  out->dump();
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
  (this->*op->func)(args);
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

Boolean Gfx::checkArg(Object *arg, TchkType type) {
  switch (type) {
  case tchkBool:   return arg->isBool();
  case tchkInt:    return arg->isInt();
  case tchkNum:    return arg->isNum();
  case tchkString: return arg->isString();
  case tchkName:   return arg->isName();
  case tchkArray:  return arg->isArray();
  }
  return false;
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

void Gfx::opRestore(Object args[]) {
  cover("Gfx::opRestore");
  state = state->restore();
  out->restoreState(state);
}

void Gfx::opConcat(Object args[]) {
  cover("Gfx::opConcat");
  state->concatCTM(args[0].getNum(), args[1].getNum(),
		   args[2].getNum(), args[3].getNum(),
		   args[4].getNum(), args[5].getNum());
  out->updateCTM(state);
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
    dash = (double *)smalloc(length * sizeof(double));
    for (i = 0; i < length; ++i) {
      dash[i] = a->get(i, &obj)->getNum();
      obj.free();
    }
  }
  state->setLineDash(dash, length, args[1].getNum());
  out->updateLineDash(state);
}

void Gfx::opSetFlat(Object args[]) {
  cover("Gfx::opSetFlat");
  state->setFlatness((int)args[0].getNum());
  out->updateFlatness(state);
}

void Gfx::opSetLineJoin(Object args[]) {
  cover("Gfx::opSetLineJoin");
  state->setLineJoin(args[0].getInt());
  out->updateLineJoin(state);
}

void Gfx::opSetLineCap(Object args[]) {
  cover("Gfx::opSetLineCap");
  state->setLineCap(args[0].getInt());
  out->updateLineCap(state);
}

void Gfx::opSetMiterLimit(Object args[]) {
  cover("Gfx::opSetMiterLimit");
  state->setMiterLimit(args[0].getNum());
  out->updateMiterLimit(state);
}

void Gfx::opSetLineWidth(Object args[]) {
  cover("Gfx::opSetLineWidth");
  state->setLineWidth(args[0].getNum());
  out->updateLineWidth(state);
}

//------------------------------------------------------------------------
// color operators
//------------------------------------------------------------------------

void Gfx::opSetFillGray(Object args[]) {
  cover("Gfx::opSetFillGray");
  state->setFillGray(args[0].getNum());
  out->updateFillColor(state);
}

void Gfx::opSetStrokeGray(Object args[]) {
  cover("Gfx::opSetStrokeGray");
  state->setStrokeGray(args[0].getNum());
  out->updateStrokeColor(state);
}

void Gfx::opSetFillCMYKColor(Object args[]) {
  cover("Gfx::opSetFillCMYKColor");
  state->setFillCMYK(args[0].getNum(), args[1].getNum(),
		     args[2].getNum(), args[3].getNum());
  out->updateFillColor(state);
}

void Gfx::opSetStrokeCMYKColor(Object args[]) {
  cover("Gfx::opSetStrokeCMYKColor");
  state->setStrokeCMYK(args[0].getNum(), args[1].getNum(),
		       args[2].getNum(), args[3].getNum());
  out->updateStrokeColor(state);
}

void Gfx::opSetFillRGBColor(Object args[]) {
  cover("Gfx::opSetFillRGBColor");
  state->setFillRGB(args[0].getNum(), args[1].getNum(), args[2].getNum());
  out->updateFillColor(state);
}

void Gfx::opSetStrokeRGBColor(Object args[]) {
  cover("Gfx::opSetStrokeRGBColor");
  state->setStrokeRGB(args[0].getNum(), args[1].getNum(), args[2].getNum());
  out->updateStrokeColor(state);
}

//------------------------------------------------------------------------
// path segment operators
//------------------------------------------------------------------------

void Gfx::opMoveTo(Object args[]) {
  cover("Gfx::opMoveTo");
  state->moveTo(args[0].getNum(), args[1].getNum());
}

void Gfx::opLineTo(Object args[]) {
  cover("Gfx::opLineTo");
  if (!state->isPath()) {
    error(0, "No current point in lineto");
    return;
  }
  state->lineTo(args[0].getNum(), args[1].getNum());
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

//------------------------------------------------------------------------
// path painting operators
//------------------------------------------------------------------------

void Gfx::opEndPath(Object args[]) {
  cover("Gfx::opEndPath");
  doEndPath();
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

void Gfx::opFill(Object args[]) {
  cover("Gfx::opFill");
  if (!state->isPath()) {
    error(0, "No path in fill");
    return;
  }
  out->fill(state);
  doEndPath();
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

void Gfx::doEndPath() {
  if (clip == clipNormal)
    out->clip(state);
  else if (clip == clipEO)
    out->eoClip(state);
  clip = clipNone;
  state->clearPath();
}

//------------------------------------------------------------------------
// path clipping operators
//------------------------------------------------------------------------

void Gfx::opClip(Object args[]) {
  cover("Gfx::opClip");
  clip = clipNormal;
}

void Gfx::opEOClip(Object args[]) {
  cover("Gfx::opEOClip");
  clip = clipEO;
}

//------------------------------------------------------------------------
// text object operators
//------------------------------------------------------------------------

void Gfx::opBeginText(Object args[]) {
  cover("Gfx::opBeginText");
  state->setTextMat(1, 0, 0, 1, 0, 0);
  state->textMoveTo(0, 0);
  fontChanged = true;
}

void Gfx::opEndText(Object args[]) {
  cover("Gfx::opEndText");
}

//------------------------------------------------------------------------
// text state operators
//------------------------------------------------------------------------

void Gfx::opSetCharSpacing(Object args[]) {
  cover("Gfx::opSetCharSpacing");
  state->setCharSpace(args[0].getNum());
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
	   font->getName()->getCString(), args[1].getNum());
  }
  state->setFont(font, args[1].getNum());
  fontChanged = true;
}

void Gfx::opSetTextLeading(Object args[]) {
  cover("Gfx::opSetTextLeading");
  state->setLeading(args[0].getNum());
}

void Gfx::opSetTextRender(Object args[]) {
  cover("Gfx::opSetTextRender");
  state->setRender(args[0].getInt());
}

void Gfx::opSetTextRise(Object args[]) {
  cover("Gfx::opSetTextRise");
  state->setRise(args[0].getNum());
}

void Gfx::opSetWordSpacing(Object args[]) {
  cover("Gfx::opSetWordSpacing");
  state->setWordSpace(args[0].getNum());
}

void Gfx::opSetHorizScaling(Object args[]) {
  cover("Gfx::opSetHorizScaling");
  state->setHorizScaling(args[0].getNum());
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

void Gfx::opTextMoveSet(Object args[]) {
  double tx, ty;

  cover("Gfx::opTextMoveSet");
  tx = state->getLineX() + args[0].getNum();
  ty = args[1].getNum();
  state->setLeading(-ty);
  ty += state->getLineY();
  state->textMoveTo(tx, ty);
}

void Gfx::opSetTextMatrix(Object args[]) {
  cover("Gfx::opSetTextMatrix");
  state->setTextMat(args[0].getNum(), args[1].getNum(),
		    args[2].getNum(), args[3].getNum(),
		    args[4].getNum(), args[5].getNum());
  state->textMoveTo(0, 0);
  fontChanged = true;
}

void Gfx::opTextNextLine(Object args[]) {
  double tx, ty;

  cover("Gfx::opTextNextLine");
  tx = state->getLineX();
  ty = state->getLineY() - state->getLeading();
  state->textMoveTo(tx, ty);
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
  doShowText((uchar *)args[0].getString());
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
  doShowText((uchar *)args[0].getString());
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
  doShowText((uchar *)args[2].getString());
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
      doShowText((uchar *)obj.getString());
    else
      error(0, "Element of show/space array must be number or string");
    obj.free();
  }
}

void Gfx::doShowText(uchar *s) {
  uchar *p;
  double dx, dy;

  if (fontChanged) {
    out->updateFont(state);
    fontChanged = false;
  }
  state->textTransformDelta(0, state->getRise(), &dx, &dy);
  for (p = s; *p; ++p) {
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

void Gfx::doImage(Stream *str) {
  Dict *dict;
  Object obj1, obj2;
  int width, height;
  int bits;
  Boolean mask;
  GfxColorSpace *colorSpace;
  Boolean invert;

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
  mask = false;
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
    invert = false;
    dict->lookup("Decode", &obj1);
    if (obj1.isNull()) {
      obj1.free();
      dict->lookup("D", &obj1);
    }
    if (obj1.isArray()) {
      obj1.arrayGet(0, &obj2);
      if (obj2.isInt() && obj2.getInt() == 1)
	invert = true;
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

void Gfx::doForm(Stream *str) {
  cover("Gfx::doForm");
  error(0, "Unimplemented: form");
}

//------------------------------------------------------------------------
// in-line image operators
//------------------------------------------------------------------------

void Gfx::opBeginImage(Object args[]) {
  Object dict;
  Object obj;
  char *key;
  Stream *str;

  cover("Gfx::opBeginImage");

  // inline image must be in stream
  if (!parser) {
    error(0, "Inline image in array content stream");
    return;
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

  // display the image
  doImage(str);
  delete str;

  // get 'EI' tag
  nextObj(&obj);
  if (!obj.isCmd("EI"))
    error(0, "Expected 'EI' operator");
  obj.free();
}

void Gfx::opImageData(Object args[]) {
  error(0, "Internal: got 'ID' operator");
}

void Gfx::opEndImage(Object args[]) {
  error(0, "Internal: got 'EI'operator");
}

//------------------------------------------------------------------------
// type 3 font operators
//------------------------------------------------------------------------

void Gfx::opSetCharWidth(Object args[]) {
  cover("Gfx::opSetCharWidth");
  error(0, "Encountered 'd0' operator in content stream");
}

void Gfx::opSetCacheDevice(Object args[]) {
  cover("Gfx::opSetCacheDevice");
  error(0, "Encountered 'd1' operator in content stream");
}
