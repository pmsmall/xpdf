//========================================================================
//
// Page.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#pragma implementation

#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "OutputDev.h"
#include "Gfx.h"
#include "Error.h"
#include "Flags.h"
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
  } else {
    x1 = y1 = x2 = y2 = 0;
    rotate = 0;
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
}

//------------------------------------------------------------------------
// Page
//------------------------------------------------------------------------

Page::Page(Dict *pageDict, PageAttrs *attrs1, int pageNum) {
  Object resourceDict;

  ok = true;

  // get attributes
  attrs = attrs1;

  // resources
  pageDict->lookup("Resources", &resourceDict);
  if (resourceDict.isDict()) {
    resourceDict.dictLookup("Font", &fontDict);
    if (!(fontDict.isDict() || fontDict.isNull())) {
      error(0, "Font resources object (page %d) is wrong type (%s)",
	    pageNum, fontDict.getTypeName());
      goto err2;
    }
    resourceDict.dictLookup("XObject", &xObjDict);
    if (!(xObjDict.isDict() || xObjDict.isNull())) {
      error(0, "XObject resources object (page %d) is wrong type (%s)",
	    pageNum, xObjDict.getTypeName());
      goto err2;
    }
  } else if (resourceDict.isNull()) {
    fontDict.initNull();
    xObjDict.initNull();
  } else {
    error(0, "Resources object (page %d) is wrong type (%s)",
	  pageNum, resourceDict.getTypeName());
    goto err2;
  }
  resourceDict.free();

  // contents
  pageDict->lookupNF("Contents", &contents);
  if (!(contents.isRef() || contents.isArray() ||
	contents.isNull())) {
    error(0, "Page contents object (page %d) is wrong type (%s)",
	  pageNum, contents.getTypeName());
    goto err1;
  }

  return;

 err2:
  resourceDict.free();
 err1:
  ok = false;
}

Page::~Page() {
  delete attrs;
  fontDict.free();
  xObjDict.free();
  contents.free();
}

void Page::display(OutputDev *out, int dpi, int rotate) {
  Gfx *gfx;
  Dict *fonts;
  Dict *xObjects;
  Object obj;
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
  gfx = new Gfx(out, fonts, xObjects, dpi, attrs->getX1(), attrs->getY1(),
		attrs->getX2(), attrs->getY2(), rotate);
  if (contents.isArray()) {
    for (i = 0; i < contents.arrayGetLength(); ++i) {
      contents.arrayGet(i, &obj);
      if (obj.isArray())
	gfx->display(obj.getArray());
      else if (obj.isStream())
	gfx->display(obj.getStream());
      obj.free();
    }
  } else if (contents.isRef()) {
    contents.fetch(&obj);
    if (obj.isArray())
      gfx->display(obj.getArray());
    else if (obj.isStream())
      gfx->display(obj.getStream());
    obj.free();
  }
  delete gfx;
}

void Page::print(FILE *f) {
  fprintf(f, "<<\n");
  fprintf(f, "  /Type /Page\n");
  fprintf(f, "  /MediaBox [%d %d %d %d]\n",
	  attrs->getX1(), attrs->getY1(), attrs->getX2(), attrs->getY2());
  fprintf(f, "  /Rotate %d\n", attrs->getRotate());
  fprintf(f, "  /Contents ");
  contents.print(f);
  fprintf(f, "\n");
  fprintf(f, ">>\n");
}
