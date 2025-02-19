//========================================================================
//
// Catalog.h
//
//========================================================================

#ifndef CATALOG_H
#define CATALOG_H

#pragma interface

#include <stdio.h>

class XRef;
class Page;
class PageAttrs;

//------------------------------------------------------------------------
// Catalog
//------------------------------------------------------------------------

class Catalog {
public:

  // Constructor.
  Catalog(Object *catDict);

  // Destructor.
  ~Catalog();

  // Is catalog valid?
  Boolean isOk() { return ok; }

  // Get number of pages.
  int getNumPages() { return numPages; }

  // Get a page.
  Page *getPage(int i) { return pages[i-1]; }

  // Output.
  void print(FILE *f = stdout);

private:

  XRef *xref;			// the document xref table
  Page **pages;			// array of pages
  int numPages;			// number of pages
  Boolean ok;			// true if catalog is valid

  int readPageTree(Dict *pages, PageAttrs *attrs, int start);
};

#endif
