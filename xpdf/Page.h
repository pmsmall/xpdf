//========================================================================
//
// Page.h
//
//========================================================================

#ifndef PAGE_H
#define PAGE_H

#pragma interface

#include <stdio.h>
#include "Object.h"

class Dict;
class XRef;
class OutputDev;
class Gfx;

//------------------------------------------------------------------------
// PageAttrs
//------------------------------------------------------------------------

class PageAttrs {
public:

  // Construct a new PageAttrs object by merging a dictionary
  // (of type Pages or Page) into another PageAttrs object.  If
  // <attrs> is NULL, uses defaults.
  PageAttrs(PageAttrs *attrs, Dict *dict);

  // Accessors.
  int getX1() { return x1; }
  int getY1() { return y1; }
  int getX2() { return x2; }
  int getY2() { return y2; }
  int getRotate() { return rotate; }

private:

  int x1, y1, x2, y2;
  int rotate;
};

//------------------------------------------------------------------------
// Page
//------------------------------------------------------------------------

class Page {
public:

  // Constructor.
  Page(Dict *pageDict, PageAttrs *attrs1, int pageNum);

  // Destructor.
  ~Page();

  // Is page valid?
  Boolean isOk() { return ok; }

  // Get page size.
  int getWidth() { return attrs->getX2() - attrs->getX1(); }
  int getHeight() { return attrs->getY2() - attrs->getY1(); }

  // Display a page.
  void display(OutputDev *out, int dpi, int rotate);

  // Output.
  void print(FILE *f = stdout);

private:

  PageAttrs *attrs;		// page attributes
  Object fontDict;		// font dictionary
  Object xObjDict;		// XObject dictionary
  Object contents;		// page contents
  Boolean ok;			// true if page is valid
};

#endif
