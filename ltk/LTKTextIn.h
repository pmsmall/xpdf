//========================================================================
//
// LTKTextIn.h
//
// Copyright 1996-2002 Glyph & Cog, LLC
//
//========================================================================

#ifndef LTKTEXTIN_H
#define LTKTEXTIN_H

#ifdef __GNUC__
#pragma interface
#endif

#include "gtypes.h"
#include "GString.h"
#include "LTKWidget.h"

//------------------------------------------------------------------------
// LTKTextIn
//------------------------------------------------------------------------

class LTKTextIn: public LTKWidget {
public:

  //---------- constructor and destructor ----------

  LTKTextIn(char *nameA, int widgetNumA, int minWidthA,
	    char *fontNameA, LTKStringValCbk doneCbkA,
	    char *tabTargetA);

  virtual ~LTKTextIn();

  //---------- access ----------

  virtual long getEventMask();

  //---------- special access ----------

  GString *getText() { return text; }
  void setText(char *s);
  void selectAll();

  //---------- layout ----------

  virtual void layout1();
  virtual void layout3();

  //---------- drawing ----------

  virtual void redraw();

  //---------- callbacks and event handlers ----------

  virtual void buttonPress(int mx, int my, int button, GBool dblClick);
  virtual void buttonRelease(int mx, int my, int button, GBool click);
  virtual void mouseMove(int mx, int my, int btn);
  virtual void activate(GBool on);
  virtual void keyPress(KeySym key, Guint modifiers, char *s, int n);
  virtual void clearSelection();
  virtual void paste(GString *str);

protected:

  int xToCursor(int mx);
  int cursorToX(int cur);
  void xorCursor();
  void moveCursor(int newCursor, int newSelectionEnd,
		  int visiblePos);
  void redrawTail(int i);

  int minWidth;			// minimum width
  GString *text;		// the current text
  GBool active;			// set if widget has input focus
  int firstChar;		// index of first displayed char
  int cursor;			// cursor is before char #<cursor>
  int selectionEnd;		// end of selection
				//   (<= cursor means no selection)
  GBool dragging;		// set while button1 is pressed
  int dragAnchor;		// position where drag started
  int textHeight;		// height of text
  int textBase;			// baseline offset

  LTKStringValCbk doneCbk;	// called when <Return> is pressed or
				//   widget is de-selected
  char *tabTarget;		// name of widget to be activated when
				//   <Tab> or <Return> is pressed

  char *fontName;		// non-NULL if using a custom font
  XFontStruct *fontStruct;	// font info
  GC textGC;			// GC with text font
};

#endif
