//========================================================================
//
// Array.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#pragma implementation

#include <mem.h>
#include "Object.h"
#include "Array.h"

//------------------------------------------------------------------------
// Array
//------------------------------------------------------------------------

Array::Array() {
  elems = NULL;
  size = length = 0;
  ref = 1;
}

Array::~Array() {
  int i;

  for (i = 0; i < length; ++i)
    elems[i].free();
  sfree(elems);
}

void Array::add(Object *elem) {
  if (length + 1 > size) {
    size += 8;
    if (elems)
      elems = (Object *)srealloc(elems, size * sizeof(Object));
    else
      elems = (Object *)smalloc(size * sizeof(Object));
  }
  elems[length] = *elem;
  ++length;
}

Object *Array::get(int i, Object *obj) {
  return elems[i].fetch(obj);
}

Object *Array::getNF(int i, Object *obj) {
  return elems[i].copy(obj);
}

void Array::print(FILE *f) {
  int i;

  fprintf(f, "[");
  for (i = 0; i < length; ++i) {
    if (i > 0)
      fputc(' ', f);
    elems[i].print(f);
  }
  fprintf(f, "]");
}
