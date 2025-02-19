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
    cropX1 = attrs->cropX1;
    cropY1 = attrs->cropY1;
    cropX2 = attrs->cropX2;
    cropY2 = attrs->cropY2;
    rotate = attrs->rotate;
    attrs->resources.copy(&resources);
  } else {
    // set default MediaBox to 8.5" x 11" -- this shouldn't be necessary
    // but some (non-compliant) PDF files don't specify a MediaBox
    x1 = 0;
    y1 = 0;
    x2 = 612;
    y2 = 792;
    cropX1 = cropY1 = cropX2 = cropY2 = 0;
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

  // crop box
  dict->lookup("CropBox", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 4) {
    obj1.arrayGet(0, &obj2);
    if (obj2.isInt())
      cropX1 = obj2.getInt();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    if (obj2.isInt())
      cropY1 = obj2.getInt();
    obj2.free();
    obj1.arrayGet(2, &obj2);
    if (obj2.isInt())
      cropX2 = obj2.getInt();
    obj2.free();
    obj1.arrayGet(3, &obj2);
    if (obj2.isInt())
      cropY2 = obj2.getInt();
    obj2.free();
  } else {
    cropX1 = cropX2 = cropY1 = cropY2 = 0;
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

  ok = gTrue;
  num = num1;

  // get attributes
  attrs = attrs1;

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

 err2:
  annots.initNull();
 err1:
  contents.initNull();
  ok = gFalse;
}

Page::~Page() {
  delete attrs;
  annots.free();
  contents.free();
}

Object *Page::getFontDict(Object *obj) {
  Dict *resDict;

  if ((resDict = attrs->getResourceDict()))
    resDict->lookup("Font", obj);
  else
    obj->initNull();
  return obj;
}

void Page::display(OutputDev *out, int dpi, int rotate) {
  Gfx *gfx;
  Object obj;

  if (printCommands) {
    printf("***** MediaBox = ll:%d,%d ur:%d,%d\n",
	   getX1(), getY1(), getX2(), getY2());
    if (isCropped()) {
      printf("***** CropBox = ll:%d,%d ur:%d,%d\n",
	     getCropX1(), getCropY1(), getCropX2(), getCropY2());
    }
    printf("***** Rotate = %d\n", attrs->getRotate());
  }
  rotate += getRotate();
  if (rotate >= 360)
    rotate -= 360;
  else if (rotate < 0)
    rotate += 360;
  gfx = new Gfx(out, num, attrs->getResourceDict(),
		dpi, getX1(), getY1(), getX2(), getY2(), isCropped(),
		getCropX1(), getCropY1(), getCropX2(), getCropY2(), rotate);
  contents.fetch(&obj);
  if (!obj.isNull())
    gfx->display(&obj);
  obj.free();
  delete gfx;
}
