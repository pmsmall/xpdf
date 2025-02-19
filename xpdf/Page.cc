//========================================================================
//
// Page.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stddef.h>
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "OutputDev.h"
#include "Gfx.h"
#include "Error.h"
#include "Params.h"
#include "Page.h"

//------------------------------------------------------------------------
// PageAttrs
//------------------------------------------------------------------------

PageAttrs::PageAttrs(PageAttrs *attrs, Dict *dict) {
  Object obj1, obj2;

  // get old/default values
  if (attrs) {
    x1 = attrs->x1;
    y1 = attrs->y1;
    x2 = attrs->x2;
    y2 = attrs->y2;
    rotate = attrs->rotate;
    attrs->resources.copy(&resources);
  } else {
    // set default MediaBox to 8.5" x 11" -- this shouldn't be necessary
    // but some (non-compliant) PDF files don't specify a MediaBox
    x1 = 0;
    y1 = 0;
    x2 = 612;
    y2 = 792;
    rotate = 0;
    resources.initNull();
  }

  // media box
  dict->lookup("MediaBox", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 4) {
    obj1.arrayGet(0, &obj2);
    if (obj2.isInt())
      x1 = obj2.getInt();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    if (obj2.isInt())
      y1 = obj2.getInt();
    obj2.free();
    obj1.arrayGet(2, &obj2);
    if (obj2.isInt())
      x2 = obj2.getInt();
    obj2.free();
    obj1.arrayGet(3, &obj2);
    if (obj2.isInt())
      y2 = obj2.getInt();
    obj2.free();
  }
  obj1.free();

  // rotate
  dict->lookup("Rotate", &obj1);
  if (obj1.isInt())
    rotate = obj1.getInt();
  obj1.free();
  while (rotate < 0)
    rotate += 360;
  while (rotate >= 360)
    rotate -= 360;

  // resource dictionary
  dict->lookup("Resources", &obj1);
  if (obj1.isDict()) {
    resources.free();
    obj1.copy(&resources);
  }
  obj1.free();
}

PageAttrs::~PageAttrs() {
  resources.free();
}

//------------------------------------------------------------------------
// Page
//------------------------------------------------------------------------

Page::Page(int num1, Dict *pageDict, PageAttrs *attrs1) {
  Dict *resourceDict;

  ok = gTrue;
  num = num1;

  // get attributes
  attrs = attrs1;

  // resources
  if ((resourceDict = attrs->getResourceDict())) {
    resourceDict->lookup("Font", &fontDict);
    if (!(fontDict.isDict() || fontDict.isNull())) {
      error(-1, "Font resources object (page %d) is wrong type (%s)",
	    num, fontDict.getTypeName());
      fontDict.free();
      goto err4;
    }
    resourceDict->lookup("XObject", &xObjDict);
    if (!(xObjDict.isDict() || xObjDict.isNull())) {
      error(-1, "XObject resources object (page %d) is wrong type (%s)",
	    num, xObjDict.getTypeName());
      xObjDict.free();
      goto err3;
    }
  } else {
    fontDict.initNull();
    xObjDict.initNull();
  }

  // annotations
  pageDict->lookupNF("Annots", &annots);
  if (!(annots.isRef() || annots.isArray() || annots.isNull())) {
    error(-1, "Page annotations object (page %d) is wrong type (%s)",
	  num, annots.getTypeName());
    annots.free();
    goto err2;
  }

  // contents
  pageDict->lookupNF("Contents", &contents);
  if (!(contents.isRef() || contents.isArray() ||
	contents.isNull())) {
    error(-1, "Page contents object (page %d) is wrong type (%s)",
	  num, contents.getTypeName());
    contents.free();
    goto err1;
  }

  return;

 err4:
  fontDict.initNull();
 err3:
  xObjDict.initNull();
 err2:
  annots.initNull();
 err1:
  contents.initNull();
  ok = gFalse;
}

Page::~Page() {
  delete attrs;
  fontDict.free();
  xObjDict.free();
  annots.free();
  contents.free();
}

void Page::display(OutputDev *out, int dpi, int rotate) {
  Gfx *gfx;
  Dict *fonts;
  Dict *xObjects;
  Object obj1, obj2;
  int i;

  if (printCommands) {
    printf("***** MediaBox = ll:%d,%d ur:%d,%d\n",
	   attrs->getX1(), attrs->getY1(), attrs->getX2(), attrs->getY2());
    printf("***** Rotate = %d\n", attrs->getRotate());
  }
  if (fontDict.isDict())
    fonts = fontDict.getDict();
  else
    fonts = NULL;
  if (xObjDict.isDict())
    xObjects = xObjDict.getDict();
  else
    xObjects = NULL;
  rotate += attrs->getRotate();
  if (rotate >= 360)
    rotate -= 360;
  else if (rotate < 0)
    rotate += 360;
  gfx = new Gfx(out, num, fonts, xObjects, dpi, attrs->getX1(), attrs->getY1(),
		attrs->getX2(), attrs->getY2(), rotate);
  contents.fetch(&obj1);
  if (obj1.isArray()) {
    for (i = 0; i < obj1.arrayGetLength(); ++i) {
      obj1.arrayGet(i, &obj2);
      if (obj2.isStream())
	gfx->display(obj2.getStream());
      else
	error(-1, "Weird page contents");
      obj2.free();
    }
  } else if (obj1.isStream()) {
    gfx->display(obj1.getStream());
  } else {
    error(-1, "Weird page contents");
  }
  obj1.free();
  delete gfx;
}
