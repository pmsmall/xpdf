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

//------------------------------------------------------------------------
// PageAttrs
//------------------------------------------------------------------------

class PageAttrs {
public:

  // Construct a new PageAttrs object by merging a dictionary
  // (of type Pages or Page) into another PageAttrs object.  If
  // <attrs> is NULL, uses defaults.
  PageAttrs(PageAttrs *attrs, Dict *dict);

  // Destructor.
  ~PageAttrs();

  // Accessors.
  int getX1() { return x1; }
  int getY1() { return y1; }
  int getX2() { return x2; }
  int getY2() { return y2; }
  GBool isCropped() { return cropX1 < cropX2; }
  int getCropX1() { return cropX1; }
  int getCropY1() { return cropY1; }
  int getCropX2() { return cropX2; }
  int getCropY2() { return cropY2; }
  int getRotate() { return rotate; }
  Dict *getResourceDict()
    { return resources.isDict() ? resources.getDict() : (Dict *)NULL; }

private:

  int x1, y1, x2, y2;
  int cropX1, cropY1, cropX2, cropY2;
  int rotate;
  Object resources;
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

  // Get page parameters.
  int getX1() { return attrs->getX1(); }
  int getY1() { return attrs->getY1(); }
  int getX2() { return attrs->getX2(); }
  int getY2() { return attrs->getY2(); }
  GBool isCropped() { return attrs->isCropped(); }
  int getCropX1() { return attrs->getCropX1(); }
  int getCropY1() { return attrs->getCropY1(); }
  int getCropX2() { return attrs->getCropX2(); }
  int getCropY2() { return attrs->getCropY2(); }
  int getWidth() { return attrs->getX2() - attrs->getX1(); }
  int getHeight() { return attrs->getY2() - attrs->getY1(); }
  int getRotate() { return attrs->getRotate(); }

  // Get font dictionary.
  Object *getFontDict(Object *obj);

  // Get annotations array.
  Object *getAnnots(Object *obj) { return annots.fetch(obj); }

  // Display a page.
  void display(OutputDev *out, int dpi, int rotate);

private:

  int num;			// page number
  PageAttrs *attrs;		// page attributes
  Object annots;		// annotations array
  Object contents;		// page contents
  GBool ok;			// true if page is valid
};

#endif
