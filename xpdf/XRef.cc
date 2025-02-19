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
// Permission bits
//------------------------------------------------------------------------

#define permPrint    (1<<2)
#define permChange   (1<<3)
#define permCopy     (1<<4)
#define permNotes    (1<<5)
#define defPermFlags 0xfffc

//------------------------------------------------------------------------
// The global xref table
//------------------------------------------------------------------------

XRef *xref = NULL;

//------------------------------------------------------------------------
// XRef
//------------------------------------------------------------------------

XRef::XRef(FileStream *str) {
  XRef *oldXref;
  int pos;
  int i;

  ok = gTrue;

  // get rid of old xref (otherwise it will try to fetch the Root object
  // in the new document, using the old xref)
  oldXref = xref;
  xref = NULL;

  entries = NULL;
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
  xref = this;
  if (checkEncrypted()) {
    ok = gFalse;
    xref = oldXref;
    return;
  }
}

XRef::~XRef() {
  gfree(entries);
  trailerDict.free();
}

// Read startxref position, xref table size, and root.  Returns
// first xref position.
int XRef::readTrailer(FileStream *str) {
  Parser *parser;
  Object obj;
  char buf[xrefSearchSize+1];
  int n, pos, pos1;
  char *p;
  int c;
  int i;

  // read last xrefSearchSize bytes
  str->setPos(-xrefSearchSize);
  for (n = 0; n < xrefSearchSize; ++n) {
    if ((c = str->getChar()) == EOF)
      break;
    buf[n] = c;
  }
  buf[n] = '\0';

  // find startxref
  for (i = n - 9; i >= 0; --i) {
    if (!strncmp(&buf[i], "startxref", 9))
      break;
  }
  if (i < 0)
    return 0;
  for (p = &buf[i+9]; isspace(*p); ++p) ;
  pos = atoi(p);

  // find trailer dict by looking after first xref table
  //~ there should be a better way to do this -- I need to look
  //~ at the PDF 1.2 docs
  str->setPos(start + pos);
  for (i = 0; i < 4; ++i)
    buf[i] = str->getChar();
  if (strncmp(buf, "xref", 4))
    return 0;
  pos1 = pos + 4;
  while (1) {
    str->setPos(start + pos1);
    for (i = 0; i < 35; ++i) {
      if ((c = str->getChar()) == EOF)
	return 0;
      buf[i] = c;
    }
    if (!strncmp(buf, "trailer", 7))
      break;
    p = buf;
    while (isspace(*p)) ++p;
    while ('0' <= *p && *p <= '9') ++p;
    while (isspace(*p)) ++p;
    n = atoi(p);
    while ('0' <= *p && *p <= '9') ++p;
    while (isspace(*p)) ++p;
    pos1 += (p - buf) + n * 20;
  }
  pos1 += 7;

  // read trailer dict
  obj.initNull();
  parser = new Parser(new Lexer(new FileStream(file, start + pos1, -1, &obj)));
  parser->getObj(&trailerDict);
  if (trailerDict.isDict()) {
    trailerDict.dictLookup("Size", &obj);
    if (obj.isInt())
      size = obj.getInt();
    else
      pos = 0;
    obj.free();
    trailerDict.dictLookup("Root", &obj);
    if (obj.isRef()) {
      rootNum = obj.getRefNum();
      rootGen = obj.getRefGen();
    } else {
      pos = 0;
    }
    obj.free();
  } else {
    pos = 0;
  }
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
  Object obj;
  GBool encrypted;

  trailerDict.dictLookup("Encrypt", &obj);
  if ((encrypted = !obj.isNull())) {
    error(-1, "PDF file is encrypted and cannot be displayed");
    error(-1, "* Decryption support is currently not included in xpdf");
    error(-1, "* due to legal restrictions: the U.S.A. still has bogus");
    error(-1, "* export controls on cryptography software.");
  }
  obj.free();
  return encrypted;
}

GBool XRef::okToPrint() {
  return gTrue;
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
	obj3.isCmd("obj")) {
      parser->getObj(obj);
    } else {
      obj->initNull();
    }
    obj1.free();
    obj2.free();
    obj3.free();
    delete parser;
  } else {
    obj->initNull();
  }
  return obj;
}
