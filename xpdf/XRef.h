//========================================================================
//
// XRef.h
//
//========================================================================

#ifndef XREF_H
#define XREF_H

#pragma interface

#include <stdio.h>
#include <stypes.h>

class Object;
class Stream;

//------------------------------------------------------------------------
// XRef
//------------------------------------------------------------------------

struct XRefEntry {
  int offset;
  int gen;
  Boolean used;
};

class XRef {
public:

  // Constructor.  Read xref table from stream.
  XRef(Stream *str);

  // Destructor.
  ~XRef();

  // Is xref table valid?
  Boolean isOk() { return ok; }

  // Get catalog object.
  Object *getCatalog(Object *obj) { return fetch(rootNum, rootGen, obj); }

  // Fetch an indirect reference.
  Object *fetch(int num, int gen, Object *obj);

  // Output.
  void print(FILE *f = stdout);

private:

  FILE *file;			// input file
  XRefEntry *entries;		// xref entries
  int size;			// size of <entries> array
  int rootNum, rootGen;		// catalog dict
  Boolean ok;			// true if xref table is valid

  int readTrailer(Stream *str);
  Boolean readXRef(Stream *str, int *pos);
};

//------------------------------------------------------------------------
// The global xref table
//------------------------------------------------------------------------

extern XRef *xref;

#endif
