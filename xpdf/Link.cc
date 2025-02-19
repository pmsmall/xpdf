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
#include <string.h>
#include <gmem.h>
#include <GString.h>
#include "Error.h"
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "Link.h"

//------------------------------------------------------------------------
// LinkDest
//------------------------------------------------------------------------

LinkDest::LinkDest(Array *a, GBool remote) {
  Object obj1, obj2;

  // get page
  if (remote) {
    if (!a->get(0, &obj1)->isInt()) {
      error(-1, "Bad annotation destination");
      goto err2;
    }
    pageNum = obj1.getInt() + 1;
    pageRef.num = pageRef.gen = 0;
    obj1.free();
  } else {
    if (!a->getNF(0, &obj1)->isRef()) {
      error(-1, "Bad annotation destination");
      goto err2;
    }
    pageRef.num = obj1.getRefNum();
    pageRef.gen = obj1.getRefGen();
    pageNum = 0;
    obj1.free();
  }

  // get destination info
  a->get(1, &obj1);

  // XYZ link
  if (obj1.isName("XYZ")) {
    kind = destXYZ;
    a->get(2, &obj2);
    if (obj2.isNull()) {
      changeLeft = gFalse;
    } else if (obj2.isNum()) {
      changeLeft = gTrue;
      left = obj2.getNum();
    } else {
      error(-1, "Bad annotation destination position");
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
      error(-1, "Bad annotation destination position");
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
      error(-1, "Bad annotation destination position");
      goto err1;
    }
    obj2.free();

  // Fit link
  } else if (obj1.isName("Fit")) {
    kind = destFit;

  // FitH link
  } else if (obj1.isName("FitH")) {
    kind = destFitH;
    if (!a->get(2, &obj2)->isNum()) {
      error(-1, "Bad annotation destination position");
      goto err1;
    }
    top = obj2.getNum();
    obj2.free();

  // FitV link
  } else if (obj1.isName("FitV")) {
    kind = destFitV;
    if (!a->get(2, &obj2)->isNum()) {
      error(-1, "Bad annotation destination position");
      goto err1;
    }
    left = obj2.getNum();
    obj2.free();

  // FitR link
  } else if (obj1.isName("FitR")) {
    kind = destFitR;
    if (!a->get(2, &obj2)->isNum()) {
      error(-1, "Bad annotation destination position");
      goto err1;
    }
    left = obj2.getNum();
    obj2.free();
    if (!a->get(3, &obj2)->isNum()) {
      error(-1, "Bad annotation destination position");
      goto err1;
    }
    bottom = obj2.getNum();
    obj2.free();
    if (!a->get(4, &obj2)->isNum()) {
      error(-1, "Bad annotation destination position");
      goto err1;
    }
    right = obj2.getNum();
    obj2.free();
    if (!a->get(5, &obj2)->isNum()) {
      error(-1, "Bad annotation destination position");
      goto err1;
    }
    top = obj2.getNum();
    obj2.free();

  // FitB link
  } else if (obj1.isName("FitB")) {
    kind = destFitB;

  // FitBH link
  } else if (obj1.isName("FitBH")) {
    kind = destFitBH;
    if (!a->get(2, &obj2)->isNum()) {
      error(-1, "Bad annotation destination position");
      goto err1;
    }
    top = obj2.getNum();
    obj2.free();

  // FitBV link
  } else if (obj1.isName("FitBV")) {
    kind = destFitBV;
    if (!a->get(2, &obj2)->isNum()) {
      error(-1, "Bad annotation destination position");
      goto err1;
    }
    left = obj2.getNum();
    obj2.free();

  // unknown link kind
  } else {
    error(-1, "Unknown annotation destination type");
    goto err2;
  }

  obj1.free();
  ok = gTrue;
  return;

 err1:
  obj2.free();
 err2:
  obj1.free();
  ok = gFalse;
}

//------------------------------------------------------------------------
// LinkGoto
//------------------------------------------------------------------------

LinkGoto::LinkGoto(char *subtype, Object *obj) {
  Object obj1, obj2;
  GBool remote;

  fileName = NULL;
  dest = NULL;
  namedDest = NULL;

  // destination specified directly in D entry
  if (!subtype) {

    // destination array
    if (obj->isArray()) {
      dest = new LinkDest(obj->getArray(), gFalse);
      if (!dest->isOk()) {
	delete dest;
	dest = NULL;
      }

    // named destination
    } else if (obj->isName()) {
      namedDest = new GString(obj->getName());
    }

  // scan GoTo/GoToR/Launch action dictionary
  } else if (obj->isDict()) {

    // file key
    obj->dictLookup("F", &obj1);
    if (obj1.isString()) {
      fileName = obj1.getString()->copy();
    } else if (obj1.isDict()) {
      if (!obj1.dictLookup("Unix", &obj2)->isString()) {
	obj2.free();
	obj1.dictLookup("F", &obj2);
      }
      if (obj2.isString())
	fileName = obj2.getString()->copy();
      else
	error(-1, "Can't get remote file name for link");
      obj2.free();
    }
    obj1.free();

    // destination key (for GoTo and GoToR actions)
    remote = !strcmp(subtype, "GoToR");
    if (remote || !strcmp(subtype, "GoTo")) {
      obj->dictLookup("D", &obj1);
      if (obj1.isArray()) {
	dest = new LinkDest(obj1.getArray(), remote);
	if (!dest->isOk()) {
	  delete dest;
	  dest = NULL;
	}
      } else if (obj1.isString()) {
	namedDest = obj1.getString()->copy();
      }
      obj1.free();
    }
  }
}

LinkGoto::~LinkGoto() {
  if (fileName)
    delete fileName;
  if (dest)
    delete dest;
  if (namedDest)
    delete namedDest;
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

Link::Link(Dict *dict) {
  Object obj1, obj2;

  action = NULL;

  // get rectangle
  if (!dict->lookup("Rect", &obj1)->isArray()) {
    error(-1, "Annotation rectangle is wrong type");
    goto err5;
  }
  if (!obj1.arrayGet(0, &obj2)->isInt()) {
    error(-1, "Bad annotation rectangle");
    goto err4;
  }
  x1 = obj2.getInt();
  obj2.free();
  if (!obj1.arrayGet(1, &obj2)->isInt()) {
    error(-1, "Bad annotation rectangle");
    goto err4;
  }
  y1 = obj2.getInt();
  obj2.free();
  if (!obj1.arrayGet(2, &obj2)->isInt()) {
    error(-1, "Bad annotation rectangle");
    goto err4;
  }
  x2 = obj2.getInt();
  obj2.free();
  if (!obj1.arrayGet(3, &obj2)->isInt()) {
    error(-1, "Bad annotation rectangle");
    goto err4;
  }
  y2 = obj2.getInt();
  obj2.free();
  obj1.free();

  // get border
  dict->lookup("Border", &obj1);
  if (!obj1.isArray()) {
    error(-1, "Annotation border is wrong type");
    goto err5;
  }
  if (!obj1.arrayGet(2, &obj2)->isNum()) {
    error(-1, "Bad annotation border");
    goto err4;
  }
  borderW = obj2.getNum();
  obj2.free();
  obj1.free();

  // look for destination
  if (!dict->lookup("Dest", &obj1)->isNull()) {
    action = new LinkGoto(NULL, &obj1);

  // look for action
  } else {
    obj1.free();
    if (dict->lookup("A", &obj1)->isDict()) {
      obj1.dictLookup("S", &obj2);

      // GoTo / GoToR / Launch action
      if (obj2.isName("GoTo") || obj2.isName("GoToR") ||
	  obj2.isName("Launch")) {
	action = new LinkGoto(obj2.getName(), &obj1);
	obj2.free();

      // URI action
      } else if (obj2.isName("URI")) {
	obj2.free();
	obj1.dictLookup("URI", &obj2);
	if (obj2.isString()) {
	  action = new LinkURI(obj2.getString());
	} else {
	  error(-1, "Bad annotation action");
	  action = NULL;
	}
	obj2.free();

      // unknown action
      } else if (obj2.isName()) {
	action = new LinkUnknown(obj2.getName());
        obj2.free();

      // action is missing or wrong type
      } else {
	error(-1, "Bad annotation action");
	action = NULL;
      }
    } else {
      error(-1, "Missing annotation destination/action");
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

Links::Links(Object *annots) {
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
	  links[numLinks++] = new Link(obj1.getDict());
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

LinkAction *Links::find(int x, int y) {
  int i;

  for (i = 0; i < numLinks; ++i) {
    if (links[i]->inRect(x, y) && links[i]->getAction())
      return links[i]->getAction();
  }
  return NULL;
}
