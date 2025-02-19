//========================================================================
//
// PDFDoc.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef PDFDOC_H
#define PDFDOC_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stdio.h>
#include "Link.h"

class GString;
class XRef;
class Catalog;
class OutputDev;
class Links;
class LinkAction;

//------------------------------------------------------------------------
// PDFDoc
//------------------------------------------------------------------------

class PDFDoc {
public:

  PDFDoc(GString *fileName1);
  ~PDFDoc();

  // Was PDF document successfully opened?
  GBool isOk() { return ok; }

  // Get file name.
  GString *getFileName() { return fileName; }

  // Get catalog.
  Catalog *getCatalog() { return catalog; }

  // Get page parameters.
  int getPageWidth(int page) { return catalog->getPage(page)->getWidth(); }
  int getPageHeight(int page) { return catalog->getPage(page)->getHeight(); }
  int getPageRotate(int page) { return catalog->getPage(page)->getRotate(); }

  // Get number of pages.
  int getNumPages() { return catalog->getNumPages(); }

  // Display a page.
  void displayPage(OutputDev *out, int page, int zoom, int rotate,
		   GBool doLinks);

  // Display a range of pages.
  void displayPages(OutputDev *out, int firstPage, int lastPage,
		    int zoom, int rotate);

  // Find a page, given its object ID.  Returns page number, or 0 if
  // not found.
  int findPage(int num, int gen) { return catalog->findPage(num, gen); }

  // If point <x>,<y> is in a link, return the associated action;
  // else return NULL.
  LinkAction *findLink(int x, int y) { return links->find(x, y); }

  // Find a named destination.  Returns the destination object
  // (array or dictionary), or a null object if <name> is not a
  // destination.
  Object *findDest(GString *name, Object *obj)
    { return catalog->findDest(name, obj); }

  // Is printing allowed?  If not, print an error message.
  GBool okToPrint() { return xref->okToPrint(); }

private:

  Links *getLinks(int page);

  GString *fileName;
  FILE *file;
  XRef *xref;
  Catalog *catalog;
  Links *links;

  GBool ok;
};

#endif
