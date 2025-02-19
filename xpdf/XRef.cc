//========================================================================
//
// XRef.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <gmem.h>
#include "Object.h"
#include "Stream.h"
#include "Lexer.h"
#include "Parser.h"
#include "Dict.h"
#include "Error.h"
#include "XRef.h"

//------------------------------------------------------------------------

#define xrefSearchSize 1024	// read this many bytes at end of file
				//   to look for 'startxref'

//------------------------------------------------------------------------
// The global xref table
//------------------------------------------------------------------------

XRef *xref = NULL;

//------------------------------------------------------------------------
// XRef
//------------------------------------------------------------------------

XRef::XRef(FileStream *str) {
  int pos;
  int i;

  ok = gTrue;
  entries = NULL;
  encrypted = gFalse;
  file = str->getFile();
  start = str->getStart();
  pos = readTrailer(str);
  if (pos == 0) {
    ok = gFalse;
    return;
  }
  entries = (XRefEntry *)gmalloc(size * sizeof(XRefEntry));
  for (i = 0; i < size; ++i)
    entries[i].offset = -1;
  while (readXRef(str, &pos)) ;
}

XRef::~XRef() {
  gfree(entries);
}

// Read startxref position, xref table size, and root.  Returns
// first xref position.
int XRef::readTrailer(FileStream *str) {
  Parser *parser;
  Object obj, obj2;
  Dict *dict;
  char buf[xrefSearchSize+1];
  int pos;
  char *p;
  int i;

  // read last xrefSearchSize bytes
  str->setPos(-xrefSearchSize);
  for (i = 0; i < xrefSearchSize; ++i)
    buf[i] = str->getChar();
  buf[xrefSearchSize] = '\0';

  // find startxref
  for (i = xrefSearchSize - 9; i >= 0; --i) {
    if (!strncmp(&buf[i], "startxref", 9))
      break;
  }
  if (i < 0)
    return 0;
  for (p = &buf[i+9]; isspace(*p); ++p) ;
  pos = atoi(p);

  // read trailer dict
  for (--i; i >= 0; --i) {
    if (!strncmp(&buf[i], "trailer", 7))
      break;
  }
  obj.initNull();
  parser = new Parser(new Lexer(
    new FileStream(file, str->getPos() - xrefSearchSize + i + 8, -1, &obj)));
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
    dict->lookup("Encrypt", &obj2);
    encrypted = !obj2.isNull();
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
GBool XRef::readXRef(FileStream *str, int *pos) {
  Parser *parser;
  Object obj, obj2;
  int first, n, i;
  GBool more;

  // make a parser
  obj.initNull();
  parser = new Parser(new Lexer(
    new FileStream(file, start + *pos, -1, &obj)));

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
      if (entries[i].offset < 0) {
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
	  entries[i].used = gTrue;
	else if (obj.isCmd("f"))
	  entries[i].used = gFalse;
	else
	  goto err;
	obj.free();
      } else {
	parser->getObj(&obj);
	obj.free();
	parser->getObj(&obj);
	obj.free();
	parser->getObj(&obj);
	obj.free();
      }
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
    more = gTrue;
  } else {
    more = gFalse;
  }
  obj.free();
  obj2.free();

  delete parser;
  return more;

 err:
  ok = gFalse;
  obj.free();
  delete parser;
  return gFalse;
}

GBool XRef::checkEncrypted() {
  if (encrypted) {
    error(0, "PDF file is encrypted and cannot be displayed");
    error(0, "*	Please send email to devsup-person@adobe.com and ask");
    error(0, "*	them to prove that PDF is truly an open standard by");
    error(0, "*	releasing the decryption specs to developers.  Also,");
    error(0, "*	please send email to the person responsible for this");
    error(0, "*	file (webmaster@... might be a good place) and ask");
    error(0, "*	them to stop using encrypted PDF files.");
  }
  return encrypted;
}

Object *XRef::fetch(int num, int gen, Object *obj) {
  XRefEntry *e;
  Parser *parser;
  Object obj1, obj2, obj3;

  e = &entries[num];
  if (e->gen == gen && e->offset >= 0) {
    obj1.initNull();
    parser = new Parser(new Lexer(
      new FileStream(file, start + e->offset, -1, &obj1)));
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
