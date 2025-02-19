//========================================================================
//
// LTKLabel.h
//
//========================================================================

#ifndef LTKLABEL_H
#define LTKLABEL_H

#pragma interface

#include <X11/Xlib.h>
#include <stypes.h>
#include <String.h>
#include <LTKWidget.h>

class LTKLabel: public LTKWidget {
public:

  LTKLabel(char *name1, int maxLength1, char *fontName1, char *text1);

  ~LTKLabel();

  virtual LTKWidget *copy() { return new LTKLabel(this); }

  virtual long getEventMask();

  virtual void layout1();

  virtual void redraw();

  void setText(char *text1);

protected:

  LTKLabel(LTKLabel *label);

  int maxLength;		// max label length
  String *text;			// the label text
  int length;			// displayed length
  int textWidth, textHeight;	// size of text
  int textBase;			// baseline offset

  char *fontName;		// non-NULL if using a custom font
  XFontStruct *fontStruct;	// font info
  GC textGC;			// GC with text font
};

#endif
