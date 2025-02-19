//========================================================================
//
// Page.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef PAGE_H
#define PAGE_H

#ifdef __GNUC__
#pragma interface
#endif

#include "Object.h"

class Dict;
class XRef;
class OutputDev;
class PSOutput;

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
  Page(int num1, Dict *pageDict, PageAttrs *attrs1);

  // Destructor.
  ~Page();

  // Is page valid?
  GBool isOk() { return ok; }

  // Get page size.
  int getWidth() { return attrs->getX2() - attrs->getX1(); }
  int getHeight() { return attrs->getY2() - attrs->getY1(); }

  // Get font dictionary.
  Dict *getFontDict()
    { return fontDict.isDict() ? fontDict.getDict() : (Dict *)NULL; }

  // Get annotations array.
  Object *getAnnots(Object *obj) { return annots.fetch(obj); }

  // Display a page.
  void display(OutputDev *out, int dpi, int rotate);

  // Generate PostScript for a page.
  void genPostScript(PSOutput *psOut, int dpi, int rotate);

private:

  int num;			// page number
  PageAttrs *attrs;		// page attributes
  Object fontDict;		// font dictionary
  Object xObjDict;		// XObject dictionary
  Object annots;		// annotations array
  Object contents;		// page contents
  GBool ok;			// true if page is valid
};

#endif
