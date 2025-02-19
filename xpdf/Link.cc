//========================================================================
//
// Link.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stddef.h>
#include <gmem.h>
#include <GString.h>
#include "Error.h"
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "Catalog.h"
#include "Link.h"

//------------------------------------------------------------------------
// LinkGoto
//------------------------------------------------------------------------

LinkGoto::LinkGoto(Array *a, Catalog *catalog) {
  remote = gFalse;
  fileName = NULL;
  ok = getPosition(a, catalog);
}

LinkGoto::LinkGoto(GString *fileName1, Array *a, Catalog *catalog) {
  remote = gTrue;
  fileName = fileName1->copy();
  ok = getPosition(a, catalog);
}

LinkGoto::LinkGoto(Dict *fileSpec, Array *a, Catalog *catalog) {
  Object obj1;

  remote = gTrue;
  fileName = NULL;
  if (!fileSpec->lookup("Unix", &obj1)->isString()) {
    obj1.free();
    fileSpec->lookup("F", &obj1);
  }
  if (obj1.isString()) {
    fileName = obj1.getString()->copy();
  } else {
    error(0, "Can't get remote file name for link");
    ok = gFalse;
  }
  obj1.free();
  ok = ok && getPosition(a, catalog);
}

GBool LinkGoto::getPosition(Array *a, Catalog *catalog) {
  Object obj1, obj2;

  // get page
  if (!a->getNF(0, &obj1)->isRef()) {
    error(0, "Bad annotation destination");
    goto err2;
  }
  pageNum = catalog->findPage(obj1.getRefNum(), obj1.getRefGen());
  if (pageNum == 0) {
    error(0, "Bad annotation destination page number");
    goto err2;
  }
  obj1.free();

  // get destination info
  a->get(1, &obj1);

  // XYZ link
  if (obj1.isName("XYZ")) {
    kind = gotoXYZ;
    a->get(2, &obj2);
    if (obj2.isNull()) {
      changeLeft = gFalse;
    } else if (obj2.isNum()) {
      changeLeft = gTrue;
      left = obj2.getNum();
    } else {
      error(0, "Bad annotation destination position");
      goto err1;
    }
    obj2.free();
    a->get(3, &obj2);
    if (obj2.isNull()) {
      changeTop = gFalse;
    } else if (obj2.isNum()) {
      changeTop = gTrue;
      top = obj2.getNum();
    } else {
      error(0, "Bad annotation destination position");
      goto err1;
    }
    obj2.free();
    a->get(4, &obj2);
    if (obj2.isNull()) {
      changeZoom = gFalse;
    } else if (obj2.isNum()) {
      changeZoom = gTrue;
      zoom = obj2.getNum();
    } else {
      error(0, "Bad annotation destination position");
      goto err1;
    }
    obj2.free();

  // Fit link
  } else if (obj1.isName("Fit")) {
    kind = gotoFit;

  // FitH link
  } else if (obj1.isName("FitH")) {
    kind = gotoFitH;
    if (!a->get(2, &obj2)->isNum()) {
      error(0, "Bad annotation destination position");
      goto err1;
    }
    top = obj2.getNum();
    obj2.free();

  // FitV link
  } else if (obj1.isName("FitV")) {
    kind = gotoFitV;
    if (!a->get(2, &obj2)->isNum()) {
      error(0, "Bad annotation destination position");
      goto err1;
    }
    left = obj2.getNum();
    obj2.free();

  // FitR link
  } else if (obj1.isName("FitR")) {
    kind = gotoFitR;
    if (!a->get(2, &obj2)->isNum()) {
      error(0, "Bad annotation destination position");
      goto err1;
    }
    left = obj2.getNum();
    obj2.free();
    if (!a->get(3, &obj2)->isNum()) {
      error(0, "Bad annotation destination position");
      goto err1;
    }
    bottom = obj2.getNum();
    obj2.free();
    if (!a->get(4, &obj2)->isNum()) {
      error(0, "Bad annotation destination position");
      goto err1;
    }
    right = obj2.getNum();
    obj2.free();
    if (!a->get(5, &obj2)->isNum()) {
      error(0, "Bad annotation destination position");
      goto err1;
    }
    top = obj2.getNum();
    obj2.free();

  // FitB link
  } else if (obj1.isName("FitB")) {
    kind = gotoFitB;

  // FitBH link
  } else if (obj1.isName("FitBH")) {
    kind = gotoFitBH;
    if (!a->get(2, &obj2)->isNum()) {
      error(0, "Bad annotation destination position");
      goto err1;
    }
    top = obj2.getNum();
    obj2.free();

  // FitBV link
  } else if (obj1.isName("FitBV")) {
    kind = gotoFitBV;
    if (!a->get(2, &obj2)->isNum()) {
      error(0, "Bad annotation destination position");
      goto err1;
    }
    left = obj2.getNum();
    obj2.free();

  // unknown link kind
  } else {
    error(0, "Unknown annotation destination type");
    goto err2;
  }

  obj1.free();
  return gTrue;

 err1:
  obj2.free();
 err2:
  obj1.free();
  return gFalse;
}

LinkGoto::~LinkGoto() {
  if (fileName)
    delete fileName;
}

//------------------------------------------------------------------------
// LinkURI
//------------------------------------------------------------------------

LinkURI::LinkURI(GString *uri1) {
  uri = uri1->copy();
}

LinkURI::~LinkURI() {
  delete uri;
}

//------------------------------------------------------------------------
// LinkUnknown
//------------------------------------------------------------------------

LinkUnknown::LinkUnknown(char *action1) {
  action = new GString(action1);
}

LinkUnknown::~LinkUnknown() {
  delete action;
}

//------------------------------------------------------------------------
// Link
//------------------------------------------------------------------------

Link::Link(Dict *dict, Catalog *catalog) {
  Object obj1, obj2, obj3;

  action = NULL;

  // get rectangle
  if (!dict->lookup("Rect", &obj1)->isArray()) {
    error(0, "Annotation rectangle is wrong type");
    goto err5;
  }
  if (!obj1.arrayGet(0, &obj2)->isInt()) {
    error(0, "Bad annotation rectangle");
    goto err4;
  }
  x1 = obj2.getInt();
  obj2.free();
  if (!obj1.arrayGet(1, &obj2)->isInt()) {
    error(0, "Bad annotation rectangle");
    goto err4;
  }
  y1 = obj2.getInt();
  obj2.free();
  if (!obj1.arrayGet(2, &obj2)->isInt()) {
    error(0, "Bad annotation rectangle");
    goto err4;
  }
  x2 = obj2.getInt();
  obj2.free();
  if (!obj1.arrayGet(3, &obj2)->isInt()) {
    error(0, "Bad annotation rectangle");
    goto err4;
  }
  y2 = obj2.getInt();
  obj2.free();
  obj1.free();

  // get border
  dict->lookup("Border", &obj1);
  if (!obj1.isArray()) {
    error(0, "Annotation border is wrong type");
    goto err5;
  }
  if (!obj1.arrayGet(2, &obj2)->isNum()) {
    error(0, "Bad annotation border");
    goto err4;
  }
  borderW = obj2.getNum();
  obj2.free();
  obj1.free();

  // look for destination
  if (dict->lookup("Dest", &obj1)->isArray()) {
    action = new LinkGoto(obj1.getArray(), catalog);

  // look for action
  } else {
    obj1.free();
    if (dict->lookup("A", &obj1)->isDict()) {
      obj1.dictLookup("S", &obj2);

      // GoTo action
      if (obj2.isName("GoTo")) {
	obj2.free();
	if (obj1.dictLookup("D", &obj2)->isArray()) {
	  action = new LinkGoto(obj2.getArray(), catalog);
	} else {
	  error(0, "Bad annotation action");
	  action = NULL;
	}
	obj2.free();

      // GoToR action
      } else if (obj2.isName("GoToR")) {
	obj2.free();
	obj1.dictLookup("F", &obj2);
	obj1.dictLookup("D", &obj3);
	if (obj2.isString() && obj3.isArray()) {
	  action = new LinkGoto(obj2.getString(), obj3.getArray(), catalog);
	} else if (obj2.isDict() && obj3.isArray()) {
	  action = new LinkGoto(obj2.getDict(), obj3.getArray(), catalog);
	} else {
	  error(0, "Bad annotation action");
	  action = NULL;
	}
	obj2.free();
	obj3.free();

      // URI action
      } else if (obj2.isName("URI")) {
	obj2.free();
	obj1.dictLookup("URI", &obj2);
	if (obj2.isString()) {
	  action = new LinkURI(obj2.getString());
	} else {
	  error(0, "Bad annotation action");
	  action = NULL;
	}
	obj2.free();

      // unknown action
      } else if (obj2.isName()) {
	action = new LinkUnknown(obj2.getName());
        obj2.free();

      // action is missing or wrong type
      } else {
	error(0, "Bad annotation action");
	action = NULL;
      }
    } else {
      error(0, "Missing annotation destination/action");
      action = NULL;
    }
  }
  obj1.free();

  // check for bad action
  if (action && !action->isOk()) {
    delete action;
    action = NULL;
  }

  return;

 err4:
  obj2.free();
 err5:
  obj1.free();
  x1 = y1 = 1;
  x2 = y2 = 0;
}

Link::~Link() {
  if (action)
    delete action;
}

//------------------------------------------------------------------------
// Links
//------------------------------------------------------------------------

Links::Links(Object *annots, Catalog *catalog) {
  Object obj1, obj2;
  int size;
  int i;

  links = NULL;
  size = 0;
  numLinks = 0;

  if (annots->isArray()) {
    for (i = 0; i < annots->arrayGetLength(); ++i) {
      if (annots->arrayGet(i, &obj1)->isDict("Annot")) {
	if (obj1.dictLookup("Subtype", &obj2)->isName("Link")) {
	  if (numLinks >= size) {
	    size += 16;
	    links = (Link **)grealloc(links, size * sizeof(Link *));
	  }
	  links[numLinks++] = new Link(obj1.getDict(), catalog);
	}
	obj2.free();
      }
      obj1.free();
    }
  }
}

Links::~Links() {
  int i;

  for (i = 0; i < numLinks; ++i)
    delete links[i];
  gfree(links);
}

Link *Links::find(int x, int y) {
  int i;

  for (i = 0; i < numLinks; ++i) {
    if (links[i]->inRect(x, y))
      return links[i];
  }
  return NULL;
}
