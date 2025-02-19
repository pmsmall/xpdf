//========================================================================
//
// XRef.cc
//
//========================================================================

#pragma implementation

#include <stdlib.h>
#include <mem.h>
#include "Object.h"
#include "Stream.h"
#include "Lexer.h"
#include "Parser.h"
#include "Dict.h"
#include "XRef.h"

//------------------------------------------------------------------------
// The global xref table
//------------------------------------------------------------------------

XRef *xref = NULL;

//------------------------------------------------------------------------
// XRef
//------------------------------------------------------------------------

XRef::XRef(Stream *str) {
  int pos;
  int i;

  ok = true;
  file = str->getFile();
  pos = readTrailer(str);
  if (pos == 0) {
    ok = false;
    return;
  }
  entries = (XRefEntry *)smalloc(size * sizeof(XRefEntry));
  for (i = 0; i < size; ++i)
    entries[i].offset = -1;
  while (readXRef(str, &pos)) ;
}

XRef::~XRef() {
  sfree(entries);
}

// Read startxref position, xref table size, and root.  Returns
// first xref position.
int XRef::readTrailer(Stream *str) {
  Parser *parser;
  Object obj, obj2;
  Dict *dict;
  char buf[256];
  int pos;
  int i;

  // read last 255 bytes
  str->setPos(-255);
  for (i = 0; i < 255; ++i)
    buf[i] = str->getChar();
  buf[255] = '\0';

  // find startxref
  for (i = 255 - 9; i >= 0; --i) {
    if (!strncmp(&buf[i], "startxref", 9))
      break;
  }
  if (i < 0)
    return 0;
  pos = atoi(&buf[i+10]);

  // read trailer dict
  for (--i; i >= 0; --i) {
    if (!strncmp(&buf[i], "trailer", 7))
      break;
  }
  obj.initNull();
  parser = new Parser(new Lexer(
    new FileStream(file, str->getPos() - 255 + i + 8, -1, &obj)));
  parser->getObj(&obj);
  if (obj.isDict()) {
    dict = obj.getDict();
    dict->lookup("Size", &obj2);
    if (obj2.isInt())
      size = obj2.getInt();
    else
      pos = 0;
    obj2.free();
    dict->lookup("Root", &obj2);
    if (obj2.isRef()) {
      rootNum = obj2.getRefNum();
      rootGen = obj2.getRefGen();
    } else {
      pos = 0;
    }
    obj2.free();
  } else {
    pos = 0;
  }
  obj.free();
  delete parser;

  // return first xref position
  return pos;
}

// Read an xref table and the prev pointer from the trailer.
Boolean XRef::readXRef(Stream *str, int *pos) {
  Parser *parser;
  Object obj, obj2;
  int first, n, i;
  Boolean more;

  // make a parser
  obj.initNull();
  parser = new Parser(new Lexer(new FileStream(file, *pos, -1, &obj)));

  // make sure it's an xref table
  parser->getObj(&obj);
  if (!obj.isCmd("xref"))
    goto err;
  obj.free();

  // read xref
  parser->getObj(&obj);
  while (!obj.isCmd("trailer")) {
    if (!obj.isInt())
      goto err;
    first = obj.getInt();
    obj.free();
    parser->getObj(&obj);
    if (!obj.isInt())
      goto err;
    n = obj.getInt();
    obj.free();
    for (i = first; i < first + n; ++i) {
      parser->getObj(&obj);
      if (!obj.isInt())
	goto err;
      entries[i].offset = obj.getInt();
      obj.free();
      parser->getObj(&obj);
      if (!obj.isInt())
	goto err;
      entries[i].gen = obj.getInt();
      obj.free();
      parser->getObj(&obj);
      if (obj.isCmd("n"))
	entries[i].used = true;
      else if (obj.isCmd("f"))
	entries[i].used = false;
      else
	goto err;
      obj.free();
    }
    parser->getObj(&obj);
  }
  obj.free();

  // read prev pointer
  parser->getObj(&obj);
  if (!obj.isDict())
    goto err;
  obj.getDict()->lookup("Prev", &obj2);
  if (obj2.isInt()) {
    *pos = obj2.getInt();
    more = true;
  } else {
    more = false;
  }
  obj.free();
  obj2.free();

  delete parser;
  return more;

 err:
  ok = false;
  obj.free();
  delete parser;
  return false;
}

Object *XRef::fetch(int num, int gen, Object *obj) {
  XRefEntry *e;
  Parser *parser;
  Object obj1, obj2, obj3;

  e = &entries[num];
  if (e->gen == gen && e->offset >= 0) {
    obj1.initNull();
    parser = new Parser(new Lexer(new FileStream(file, e->offset, -1, &obj1)));
    parser->getObj(&obj1);
    parser->getObj(&obj2);
    parser->getObj(&obj3);
    if (obj1.isInt() && obj1.getInt() == num &&
	obj2.isInt() && obj2.getInt() == gen &&
	obj3.isCmd("obj"))
      parser->getObj(obj);
    else
      obj->initNull();
    obj1.free();
    obj2.free();
    obj3.free();
    delete parser;
  } else {
    obj->initNull();
  }
  return obj;
}

void XRef::print(FILE *f) {
  int i;

  fprintf(f, "xref\n");
  fprintf(f, "%d %d\n", 0, size);
  for (i = 0; i < size; ++i) {
    fprintf(f, "%4d: %010d %05d %c \n", i,
	    entries[i].offset, entries[i].gen, entries[i].used ? 'n' : 'f');
  }
}
