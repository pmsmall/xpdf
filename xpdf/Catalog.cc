//========================================================================
//
// Catalog.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#pragma implementation

#include <mem.h>
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "Page.h"
#include "Error.h"
#include "Catalog.h"

//------------------------------------------------------------------------
// Catalog
//------------------------------------------------------------------------

Catalog::Catalog(Object *catDict) {
  Object pagesDict;
  Object obj;
  int i;

  ok = true;
  pages = NULL;
  numPages = 0;
  if (!catDict->isDict("Catalog")) {
    error(0, "Catalog object is wrong type (%s)", catDict->getTypeName());
    goto err1;
  }
  catDict->dictLookup("Pages", &pagesDict);
  if (!pagesDict.isDict("Pages")) {
    error(0, "Top-level pages object is wrong type (%s)",
	  pagesDict.getTypeName());
    goto err2;
  }
  pagesDict.dictLookup("Count", &obj);
  if (!obj.isInt()) {
    error(0, "Page count in top-level pages object is wrong type (%s)",
	  obj.getTypeName());
    goto err3;
  }
  numPages = obj.getInt();
  obj.free();
  pages = (Page **)smalloc(numPages * sizeof(Page *));
  for (i = 0; i < numPages; ++i)
    pages[i] = NULL;
  readPageTree(pagesDict.getDict(), NULL, 0);
  pagesDict.free();
  return;

 err3:
  obj.free();
 err2:
  pagesDict.free();
 err1:
  ok = false;
}

Catalog::~Catalog() {
  int i;

  if (pages) {
    for (i = 0; i < numPages; ++i) {
      if (pages[i])
	delete pages[i];
    }
    sfree(pages);
  }
}

int Catalog::readPageTree(Dict *pagesDict, PageAttrs *attrs, int start) {
  Object kids;
  Object kid;
  PageAttrs *attrs1, *attrs2;
  Page *page;
  int i;

  attrs1 = new PageAttrs(attrs, pagesDict);
  pagesDict->lookup("Kids", &kids);
  if (!kids.isArray()) {
    error(0, "Kids object (page %d) is wrong type (%s)",
	  start+1, kids.getTypeName());
    goto err1;
  }
  for (i = 0; i < kids.arrayGetLength(); ++i) {
    kids.arrayGet(i, &kid);
    if (kid.isDict("Page")) {
      attrs2 = new PageAttrs(attrs1, kid.getDict());
      page = new Page(kid.getDict(), attrs2, start);
      if (!page->isOk()) {
	++start;
	goto err3;
      }
      pages[start++] = page;
    } else if (kid.isDict("Pages")) {
      if ((start = readPageTree(kid.getDict(), attrs1, start)) < 0)
	goto err2;
    } else {
      error(0, "Kid object (page %d) is wrong type (%s)",
	    start+1, kid.getTypeName());
      goto err2;
    }
    kid.free();
  }
  delete attrs1;
  kids.free();
  return start;

 err3:
  delete page;
 err2:
  kid.free();
 err1:
  kids.free();
  delete attrs1;
  ok = false;
  return -1;
}

void Catalog::print(FILE *f) {
  int i;

  for (i = 0; i < numPages; ++i) {
    fprintf(f, "*** page %d ***\n", i + 1);
    pages[i]->print(f);
  }
}
