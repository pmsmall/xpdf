//========================================================================
//
// XRef.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef XREF_H
#define XREF_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stdio.h>
#include <gtypes.h>

class Object;
class FileStream;

//------------------------------------------------------------------------
// XRef
//------------------------------------------------------------------------

struct XRefEntry {
  int offset;
  int gen;
  GBool used;
};

class XRef {
public:

  // Constructor.  Read xref table from stream.
  XRef(FileStream *str);

  // Destructor.
  ~XRef();

  // Is xref table valid?
  GBool isOk() { return ok; }

  // Is the PDF file encrypted?
  GBool checkEncrypted();

  // Get catalog object.
  Object *getCatalog(Object *obj) { return fetch(rootNum, rootGen, obj); }

  // Fetch an indirect reference.
  Object *fetch(int num, int gen, Object *obj);

private:

  FILE *file;			// input file
  int start;			// offset in file (to allow for garbage
				//   at beginning of file)
  XRefEntry *entries;		// xref entries
  int size;			// size of <entries> array
  int rootNum, rootGen;		// catalog dict
  GBool encrypted;		// true if file is encrypted
  GBool ok;			// true if xref table is valid

  int readTrailer(FileStream *str);
  GBool readXRef(FileStream *str, int *pos);
};

//------------------------------------------------------------------------
// The global xref table
//------------------------------------------------------------------------

extern XRef *xref;

#endif
