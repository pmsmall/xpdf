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
// LinkDest
//------------------------------------------------------------------------

enum LinkDestKind {
  destXYZ,
  destFit,
  destFitH,
  destFitV,
  destFitR,
  destFitB,
  destFitBH,
  destFitBV
};

class LinkDest {
public:

  // Build a LinkDest from the array.
  LinkDest(Array *a, GBool remote);

  // Was the LinkDest created successfully?
  GBool isOk() { return ok; }

  // Accessors.
  LinkDestKind getKind() { return kind; }
  int getPageNum() { return pageNum; }
  Ref getPageRef() { return pageRef; }
  double getLeft() { return left; }
  double getBottom() { return bottom; }
  double getRight() { return right; }
  double getTop() { return top; }
  double getZoom() { return zoom; }
  GBool getChangeLeft() { return changeLeft; }
  GBool getChangeTop() { return changeTop; }
  GBool getChangeZoom() { return changeZoom; }

private:

  LinkDestKind kind;		// destination type
  int pageNum;			// remote page number
  Ref pageRef;			// reference to page
  double left, bottom;		// position
  double right, top;
  double zoom;			// zoom factor
  GBool changeLeft, changeTop;	// for destXYZ links, which position
  GBool changeZoom;		//   components to change
  GBool ok;			// set if created successfully
};

//------------------------------------------------------------------------
// LinkGoto
//------------------------------------------------------------------------

class LinkGoto: public LinkAction {
public:

  // Build a LinkGoto from a destination array, named destination string,
  // or action dictionary.  The <subtype> string is "GoTo", "GoToR",
  // "Launch", or NULL (if there was no dictionary).
  LinkGoto(char *subtype, Object *obj);

  // Destructor.
  virtual ~LinkGoto();

  // Was the LinkGoto created successfully?
  virtual GBool isOk() { return fileName || dest || namedDest; }

  // Accessors.
  virtual LinkActionKind getKind() { return actionGoto; }
  GString *getFileName() { return fileName; }
  LinkDest *getDest() { return dest; }
  GString *getNamedDest() { return namedDest; }

private:

  GString *fileName;		// file name (NULL for local link)
  LinkDest *dest;		// regular destination (NULL for remote
				//   link with bad destination)
  GString *namedDest;		// named destination (only one of dest and
				//   and namedDest may be non-NULL)
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
  Link(Dict *dict);

  // Destructor.
  ~Link();

  // Check if point is inside the link rectangle.
  GBool inRect(int x, int y)
    { return x1 <= x && x <= x2 && y1 <= y && y <= y2; }

  // Get action.
  LinkAction *getAction() { return action; }

  // Get border corners and width.
  void getBorder(int *xa1, int *ya1, int *xa2, int *ya2, double *wa)
    { *xa1 = x1; *ya1 = y1; *xa2 = x2; *ya2 = y2; *wa = borderW; }

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
  Links(Object *annots);

  // Destructor.
  ~Links();

  // Iterate through list of links.
  int getNumLinks() { return numLinks; }
  Link *getLink(int i) { return links[i]; }

  // If point <x>,<y> is in a link, return the associated action;
  // else return NULL.
  LinkAction *find(int x, int y);

private:

  Link **links;
  int numLinks;
};

#endif
