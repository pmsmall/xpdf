//========================================================================
//
// Link.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LINK_H
#define LINK_H

#ifdef __GNUC__
#pragma interface
#endif

#include "Object.h"

class GString;
class Array;
class Dict;
class Catalog;

//------------------------------------------------------------------------
// LinkAction
//------------------------------------------------------------------------

enum LinkActionKind {
  actionGoto,			// GoTo and GoToR
  actionURI,			// URI
  actionUnknown			// anything else
};

class LinkAction {
public:

  // Destructor.
  virtual ~LinkAction() {}

  // Was the LinkAction created successfully?
  virtual GBool isOk() = 0;

  // Check link action type.
  virtual LinkActionKind getKind() = 0;
};


//------------------------------------------------------------------------
// LinkGoto
//------------------------------------------------------------------------

enum LinkGotoKind {
  gotoXYZ,
  gotoFit,
  gotoFitH,
  gotoFitV,
  gotoFitR,
  gotoFitB,
  gotoFitBH,
  gotoFitBV
};

class LinkGoto: public LinkAction {
public:

  // Build a LinkGoto from the array.
  LinkGoto(Array *a, Catalog *catalog);

  // Build a remote LinkGoto from the file name and array.
  LinkGoto(GString *fileName1, Array *a, Catalog *catalog);

  // Build a remote LinkGoto from the file specification and array.
  LinkGoto(Dict *fileSpec, Array *a, Catalog *catalog);

  // Destructor.
  virtual ~LinkGoto();

  // Was the LinkGoto created successfully?
  virtual GBool isOk() { return ok; }

  // Accessors.
  virtual LinkActionKind getKind() { return actionGoto; }
  GBool isRemote() { return remote; }
  GString *getFileName() { return fileName; }
  LinkGotoKind getDestKind() { return kind; }
  int getPageNum() { return pageNum; }
  double getLeft() { return left; }
  double getBottom() { return bottom; }
  double getRight() { return right; }
  double getTop() { return top; }
  double getZoom() { return zoom; }
  GBool getChangeLeft() { return changeLeft; }
  GBool getChangeTop() { return changeTop; }
  GBool getChangeZoom() { return changeZoom; }

private:

  GBool getPosition(Array *a, Catalog *catalog);

  GBool remote;			// true if link refers to a different file
  GString *fileName;		// remote file name
  LinkGotoKind kind;		// destination type
  int pageNum;			// destination page number
  double left, bottom;		// destination position
  double right, top;
  double zoom;			// destination zoom factor
  GBool changeLeft, changeTop;	// for destXYZ links, which position
  GBool changeZoom;		//   components to change
  GBool ok;			// set if created successfully
};

//------------------------------------------------------------------------
// LinkURI
//------------------------------------------------------------------------

class LinkURI: public LinkAction {
public:

  // Build a LinkURI given the URI.
  LinkURI(GString *uri1);

  // Destructor.
  virtual ~LinkURI();

  // Was the LinkURI created successfully?
  virtual GBool isOk() { return gTrue; }

  // Accessors.
  virtual LinkActionKind getKind() { return actionURI; }
  GString *getURI() { return uri; }

private:

  GString *uri;			// the URI
};

//------------------------------------------------------------------------
// LinkUnknown
//------------------------------------------------------------------------

class LinkUnknown: public LinkAction {
public:

  // Build a LinkUnknown given the action subtype.
  LinkUnknown(char *action1);

  // Destructor.
  virtual ~LinkUnknown();

  // Was the LinkUnknown create successfully?
  virtual GBool isOk() { return gTrue; }

  // Accessors.
  virtual LinkActionKind getKind() { return actionUnknown; }
  GString *getAction() { return action; }

private:

  GString *action;		// action subtype
};

//------------------------------------------------------------------------
// Link
//------------------------------------------------------------------------

class Link {
public:

  // Construct a link, given its dictionary.
  Link(Dict *dict, Catalog *catalog);

  // Destructor.
  ~Link();

  // Check if point is inside the link rectangle.
  GBool inRect(int x, int y)
    { return x1 <= x && x <= x2 && y1 <= y && y <= y2; }

  // Get action.
  LinkAction *getAction() { return action; }

private:

  int x1, y1;			// lower left corner
  int x2, y2;			// upper right corner
  double borderW;		// border width
  LinkAction *action;		// action
};

//------------------------------------------------------------------------
// Links
//------------------------------------------------------------------------

class Links {
public:

  // Extract links from array of annotations.
  Links(Object *annots, Catalog *catalog);

  // Destructor.
  ~Links();

  // If point <x>,<y> is in a link, return the link; else return NULL.
  Link *find(int x, int y);

private:

  Link **links;
  int numLinks;
};

#endif
