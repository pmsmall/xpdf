//========================================================================
//
// Catalog.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stddef.h>
#include <gmem.h>
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

  ok = gTrue;
  pages = NULL;
  pageRefs = NULL;
  numPages = 0;

  if (!catDict->isDict("Catalog")) {
    error(-1, "Catalog object is wrong type (%s)", catDict->getTypeName());
    goto err1;
  }

  // read page tree
  catDict->dictLookup("Pages", &pagesDict);
  if (!pagesDict.isDict("Pages")) {
    error(-1, "Top-level pages object is wrong type (%s)",
	  pagesDict.getTypeName());
    goto err2;
  }
  pagesDict.dictLookup("Count", &obj);
  if (!obj.isInt()) {
    error(-1, "Page count in top-level pages object is wrong type (%s)",
	  obj.getTypeName());
    goto err3;
  }
  numPages = obj.getInt();
  obj.free();
  pages = (Page **)gmalloc(numPages * sizeof(Page *));
  pageRefs = (Ref *)gmalloc(numPages * sizeof(Ref));
  for (i = 0; i < numPages; ++i) {
    pages[i] = NULL;
    pageRefs[i].num = -1;
    pageRefs[i].gen = -1;
  }
  readPageTree(pagesDict.getDict(), NULL, 0);
  pagesDict.free();

  // read named destination dictionary
  catDict->dictLookup("Dests", &dests);

  return;

 err3:
  obj.free();
 err2:
  pagesDict.free();
 err1:
  dests.initNull();
  ok = gFalse;
}

Catalog::~Catalog() {
  int i;

  if (pages) {
    for (i = 0; i < numPages; ++i) {
      if (pages[i])
	delete pages[i];
    }
    gfree(pages);
    gfree(pageRefs);
  }
  dests.free();
}

int Catalog::readPageTree(Dict *pagesDict, PageAttrs *attrs, int start) {
  Object kids;
  Object kid;
  Object kidRef;
  PageAttrs *attrs1, *attrs2;
  Page *page;
  int i;

  attrs1 = new PageAttrs(attrs, pagesDict);
  pagesDict->lookup("Kids", &kids);
  if (!kids.isArray()) {
    error(-1, "Kids object (page %d) is wrong type (%s)",
	  start+1, kids.getTypeName());
    goto err1;
  }
  for (i = 0; i < kids.arrayGetLength(); ++i) {
    kids.arrayGet(i, &kid);
    if (kid.isDict("Page")) {
      attrs2 = new PageAttrs(attrs1, kid.getDict());
      page = new Page(start+1, kid.getDict(), attrs2);
      if (!page->isOk()) {
	++start;
	goto err3;
      }
      pages[start] = page;
      kids.arrayGetNF(i, &kidRef);
      if (kidRef.isRef()) {
	pageRefs[start].num = kidRef.getRefNum();
	pageRefs[start].gen = kidRef.getRefGen();
      }
      kidRef.free();
      ++start;
    //~ found one PDF file where a Pages object is missing the /Type entry
    // } else if (kid.isDict("Pages")) {
    } else if (kid.isDict()) {
      if ((start = readPageTree(kid.getDict(), attrs1, start)) < 0)
	goto err2;
    } else {
      error(-1, "Kid object (page %d) is wrong type (%s)",
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
  ok = gFalse;
  return -1;
}

int Catalog::findPage(int num, int gen) {
  int i;

  for (i = 0; i < numPages; ++i) {
    if (pageRefs[i].num == num && pageRefs[i].gen == gen)
      return i + 1;
  }
  return 0;
}

Object *Catalog::findDest(GString *name, Object *obj) {
  if (dests.isDict())
    return dests.dictLookup(name->getCString(), obj);
  return obj->initNull();
}
