//========================================================================
//
// LTKFileReq.h
//
// Copyright 1997 Derek B. Noonburg
//
//========================================================================

#ifndef LTKFILEREQ_H
#define LTKFILEREQ_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include "GString.h"
#include "LTKCompoundWidget.h"

class LTKBox;
class LTKList;
class LTKScrollbar;
class LTKTextIn;

//------------------------------------------------------------------------
// LTKFileReq
//------------------------------------------------------------------------

class LTKFileReq: public LTKCompoundWidget {
public:

  //---------- constructor and destructor ----------

  LTKFileReq(char *name1, int widgetNum1,
	     LTKStringValCbk selectCbk1, char *fontName1);

  virtual ~LTKFileReq();

  //---------- special access ----------

  GString *getSelection();
  GString *getDir() { return dir->copy(); }
  void setDir(GString *dir1);

  //---------- layout ----------

  virtual void layout3();
  virtual void map();

protected:

  void makeWidgets();
  void loadDirList();
  static void clickCbk(LTKWidget *widget, int widgetNum, int line);
  static void dblClickCbk(LTKWidget *widget, int widgetNum, int line);
  static void dirNameCbk(LTKWidget *widget, int widgetNum, GString *val);
  static void hScrollCbk(LTKWidget *widget, int widgetNum, int val);
  static void vScrollCbk(LTKWidget *widget, int widgetNum, int val);

  LTKStringValCbk selectCbk;	// called when user selects a file
  char *fontName;		// font name for TextIn and List widgets
  LTKTextIn *dirName;		// directory name widget
  LTKList *list;		// list widget for directory listing
  LTKScrollbar *hScrollbar;	// horizontal scrollbar
  LTKScrollbar *vScrollbar;	// vertical scrollbar
  LTKTextIn *fileName;		// file name widget

  GString *dir;			// currently dispalyed directory
  GString *selection;		// used to build selection name
  int numDirs;			// number of directories in list
};

#endif
