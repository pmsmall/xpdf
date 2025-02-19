//========================================================================
//
// LTKList.h
//
// Copyright 1997 Derek B. Noonburg
//
//========================================================================

#ifndef LTKLIST_H
#define LTKLIST_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include "GString.h"
#include "LTKWidget.h"

//------------------------------------------------------------------------
// LTKList
//------------------------------------------------------------------------

class LTKList: public LTKWidget {
public:

  //---------- constructor and destructor ----------

  LTKList(char *nameA, int widgetNumA,
	  int minWidthA, int minLinesA,
	  GBool allowSelectionA, char *fontNameA);

  virtual ~LTKList();

  //---------- access ----------

  virtual long getEventMask();

  //---------- special access ----------

  void addLine(char *s) { insertLine(numLines, s); }
  void insertLine(int line, char *s);
  void replaceLine(int line, char *s);
  void deleteLine(int line);
  void deleteAll();
  int getSelection() { return selection; }
  void setSelection(int line);
  void clearSelection() { setSelection(-1); }
  GString *getLine(int line) { return text[line]; }
  int getNumLines() { return numLines; }
  int getTopLine() { return topLine; }
  int getDisplayedLines();
  int getMaxWidth() { return maxWidth; }
  int getHorizOffset() { return horizOffset; }
  void scrollTo(int line, int horiz);
  void makeVisible(int line);
  void setClickCbk(LTKIntValCbk cbk) { clickCbk = cbk; }
  void setDblClickCbk(LTKIntValCbk cbk) { dblClickCbk = cbk; }

  //---------- layout ----------

  virtual void layout1();
  virtual void layout3();

  //---------- drawing ----------

  virtual void redraw();

  //---------- callbacks and event handlers ----------

  virtual void buttonPress(int mx, int my, int button, GBool dblClick);

protected:

  void xorSelection();
  void redrawLine(int line);
  void redrawBelow(int line);

  int minWidth, minLines;	// minimum widget size
  GBool allowSelection;		// click selects item?
  LTKIntValCbk clickCbk;	// called when user clicks on item
  LTKIntValCbk dblClickCbk;	// called when user double-clicks on item
  GString **text;		// array of lines
  int numLines;			// number of lines
  int textSize;			// size of text array
  int topLine;			// current top line
  int horizOffset;		// horizontal scroll offset
  int selection;		// currently selected line (-1 for none)
  int maxWidth;			// max line width
  int textBase;			// baseline offset
  int textHeight;		// height of text

  char *fontName;		// non-NULL if using a custom font
  XFontStruct *fontStruct;	// font info
  GC textGC;			// GC with text font
};

#endif
