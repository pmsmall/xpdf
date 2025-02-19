//========================================================================
//
// LTKTextIn.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKTEXTIN_H
#define LTKTEXTIN_H

#ifdef __GNUC__
#pragma interface
#endif

#include <gtypes.h>
#include <GString.h>
#include <LTKWidget.h>

//------------------------------------------------------------------------
// LTKTextIn
//------------------------------------------------------------------------

class LTKTextIn: public LTKWidget {
public:

  //---------- constructors and destructor ----------

  LTKTextIn(char *name1, int widgetNum1, int minWidth1,
	    char *fontName1, LTKStringValCbk doneCbk1,
	    char *tabTarget1);

  ~LTKTextIn();

  virtual LTKWidget *copy() { return new LTKTextIn(this); }

  //---------- access ----------

  virtual long getEventMask();

  //---------- special access ----------

  char *getText() { return text->getCString(); }
  void setText(char *s);

  //---------- layout ----------

  virtual void layout1();
  virtual void layout3();

  //---------- drawing ----------

  virtual void redraw();

  //---------- callbacks and event handlers ----------

  virtual void buttonPress(int mx, int my, int button);
  virtual void activate(GBool on);
  virtual void keyPress(KeySym key, char *s, int n);

protected:

  LTKTextIn(LTKTextIn *textIn);
  void drawCursor(GBool on);
  void redrawTail(int i, GBool clear);

  int minWidth;			// minimum width
  GString *text;		// the current text
  GBool active;			// set if widget has input focus
  int firstChar;		// index of first displayed char
  int cursor;			// cursor is before char #<cursor>
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
