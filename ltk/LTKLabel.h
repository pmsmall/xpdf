//========================================================================
//
// LTKLabel.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKLABEL_H
#define LTKLABEL_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include <GString.h>
#include <LTKWidget.h>

//------------------------------------------------------------------------
// LTKLabel
//------------------------------------------------------------------------

class LTKLabel: public LTKWidget {
public:

  //---------- constructors and destructor ----------

  LTKLabel(char *name1, int widgetNum1,
	   int maxLength1, char *fontName1, char *text1);

  ~LTKLabel();

  virtual LTKWidget *copy() { return new LTKLabel(this); }

  //---------- special access ----------

  void setText(char *text1);

  //---------- layout ----------

  virtual void layout1();

  //---------- drawing ----------

  virtual void redraw();

protected:

  LTKLabel(LTKLabel *label);

  int maxLength;		// max label length
  GString *text;		// the label text
  int length;			// displayed length
  int textWidth, textHeight;	// size of text
  int textBase;			// baseline offset

  char *fontName;		// non-NULL if using a custom font
  XFontStruct *fontStruct;	// font info
  GC textGC;			// GC with text font
};

#endif
