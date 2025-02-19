//========================================================================
//
// Object.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#pragma implementation

#include <stdarg.h>
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "Error.h"
#include "Stream.h"

//------------------------------------------------------------------------
// Object
//------------------------------------------------------------------------

char *objTypeNames[numObjTypes] = {
  "boolean",
  "integer",
  "real",
  "string",
  "name",
  "null",
  "array",
  "dictionary",
  "stream",
  "ref",
  "cmd",
  "error",
  "eof",
  "none"
};

int Object::numAlloc[numObjTypes] = {};

Object *Object::initArray() {
  type = objArray;
  array = new Array();
  ++numAlloc[objArray];
  return this;
}

Object *Object::initDict() {
  type = objDict;
  dict = new Dict();
  ++numAlloc[objDict];
  return this;
}

Object *Object::initStream(Stream *stream1) {
  type = objStream;
  stream = stream1;
  ++numAlloc[objStream];
  return this;
}

Object *Object::copy(Object *obj) {
  *obj = *this;
  switch (type) {
  case objString:
    obj->string = copyString(string);
    break;
  case objName:
    obj->name = copyString(name);
    break;
  case objArray:
    array->incRef();
    break;
  case objDict:
    dict->incRef();
    break;
  case objStream:
    stream->incRef();
    break;
  case objCmd:
    obj->cmd = copyString(cmd);
    break;
  default:
    break;
  }
  ++numAlloc[type];
  return obj;
}

void Object::free() {
  switch (type) {
  case objString:
    sfree(string);
    break;
  case objName:
    sfree(name);
    break;
  case objArray:
    if (!array->decRef())
      delete array;
    break;
  case objDict:
    if (!dict->decRef())
      delete dict;
    break;
  case objStream:
    if (!stream->decRef())
      delete stream;
    break;
  case objCmd:
    sfree(cmd);
    break;
  default:
    break;
  }
  --numAlloc[type];
  type = objNone;
}

char *Object::getTypeName() {
  return objTypeNames[type];
}

void Object::print(FILE *f) {
  switch (type) {
  case objBool:
    fprintf(f, "%s", booln ? "true" : "false");
    break;
  case objInt:
    fprintf(f, "%d", intg);
    break;
  case objReal:
    fprintf(f, "%g", real);
    break;
  case objString:
    fprintf(f, "(%s)", string);
    break;
  case objName:
    fprintf(f, "/%s", name);
    break;
  case objNull:
    fprintf(f, "null");
    break;
  case objArray:
    array->print(f);
    break;
  case objDict:
    dict->print(f);
    break;
  case objStream:
    fprintf(f, "<stream>");
    break;
  case objRef:
    fprintf(f, "%d %d R", ref.num, ref.gen);
    break;
  case objCmd:
    fprintf(f, "%s", cmd);
    break;
  case objError:
    fprintf(f, "<error>");
    break;
  case objEOF:
    fprintf(f, "<EOF>");
  case objNone:
    fprintf(f, "<none>");
    break;
  }
}

void Object::memCheck(FILE *f) {
  int i;
  int t;

  t = 0;
  for (i = 0; i < numObjTypes; ++i)
    t += numAlloc[i];
  if (t > 0) {
    fprintf(f, "Allocated objects:\n");
    for (i = 0; i < numObjTypes; ++i) {
      if (numAlloc[i] > 0)
	fprintf(f, "  %-20s: %6d\n", objTypeNames[i], numAlloc[i]);
    }
  }
}
