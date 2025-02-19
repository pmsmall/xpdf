//========================================================================
//
// Dict.cc
//
//========================================================================

#pragma implementation

#include <mem.h>
#include "Object.h"
#include "XRef.h"
#include "Dict.h"

//------------------------------------------------------------------------
// Dict
//------------------------------------------------------------------------

Dict::Dict() {
  entries = NULL;
  size = length = 0;
  ref = 1;
}

Dict::~Dict() {
  int i;

  for (i = 0; i < length; ++i) {
    sfree(entries[i].key);
    entries[i].val.free();
  }
  sfree(entries);
}

void Dict::add(char *key, Object *val) {
  if (length + 1 > size) {
    size += 8;
    if (entries)
      entries = (DictEntry *)srealloc(entries, size * sizeof(DictEntry));
    else
      entries = (DictEntry *)smalloc(size * sizeof(DictEntry));
  }
  entries[length].key = key;
  entries[length].val = *val;
  ++length;
}

Object *Dict::lookup(char *key, Object *obj) {
  DictEntry *e;

  return (e = find(key)) ? e->val.fetch(obj) : obj->initNull();
}

Object *Dict::lookupNF(char *key, Object *obj) {
  DictEntry *e;

  return (e = find(key)) ? e->val.copy(obj) : obj->initNull();
}

char *Dict::getKey(int i) {
  return entries[i].key;
}

Object *Dict::getVal(int i, Object *obj) {
  return entries[i].val.fetch(obj);
}

Object *Dict::getValNF(int i, Object *obj) {
  return entries[i].val.copy(obj);
}

__inline DictEntry *Dict::find(char *key) {
  int i;

  for (i = 0; i < length; ++i) {
    if (!strcmp(key, entries[i].key))
      return &entries[i];
  }
  return NULL;
}

void Dict::print(FILE *f) {
  int i;

  fprintf(f, "<<");
  for (i = 0; i < length; ++i) {
    fprintf(f, " /%s ", entries[i].key);
    entries[i].val.print(f);
  }
  fprintf(f, " >>");
}
