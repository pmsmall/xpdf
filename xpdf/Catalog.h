//========================================================================
//
// Catalog.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef CATALOG_H
#define CATALOG_H

#ifdef __GNUC__
#pragma interface
#endif

class Object;
class Page;
class PageAttrs;
class Ref;

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
  GBool isOk() { return ok; }

  // Get number of pages.
  int getNumPages() { return numPages; }

  // Get a page.
  Page *getPage(int i) { return pages[i-1]; }

  // Find a page, given its object ID.  Returns page number, or 0 if
  // not found.
  int findPage(int num, int gen);

  // Find a named destination.  Returns the destination object
  // (array or dictionary), or a null object if <name> is not a
  // destination.
  Object *findDest(GString *name, Object *obj);

private:

  Page **pages;			// array of pages
  Ref *pageRefs;		// object ID for each page
  int numPages;			// number of pages
  Object dests;			// named destination dictionary
  GBool ok;			// true if catalog is valid

  int readPageTree(Dict *pages, PageAttrs *attrs, int start);
};

#endif
