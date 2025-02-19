//========================================================================
//
// LTKTextIn.h
//
//========================================================================

#ifndef LTKTEXTIN_H
#define LTKTEXTIN_H

#pragma interface

#include <stypes.h>
#include <String.h>
#include <LTKWidget.h>

class LTKTextIn;
typedef void (*LTKTextInCbk)(LTKTextIn *textIn, int widgetNum, String *text);

class LTKTextIn: public LTKWidget {
public:

  LTKTextIn(char *name1, int maxLength1, char *fontName1,
	    LTKTextInCbk cbk1, int widgetNum1);

  ~LTKTextIn();

  virtual LTKWidget *copy() { return new LTKTextIn(this); }

  virtual long getEventMask();

  virtual void layout1();

  virtual void redraw();

  virtual void buttonPress(int mx, int my, int button);
  virtual void activate(Boolean on);
  virtual void keyPress(KeySym key, char *s, int n);

  char *getText() { return text->getCString(); }
  void setText(char *s);

protected:

  LTKTextIn(LTKTextIn *textIn);
  void drawCursor(Boolean on);
  void redrawTail(int i);

  int maxLength;		// max text length
  String *text;			// the current text
  Boolean active;		// set if widget has input focus
  int cursor;			// cursor is before char #<cursor>
  LTKTextInCbk cbk;		// called when <Return> is pressed
  int widgetNum;		// widget number (for callback)
  int textWidth, textHeight;	// size of text
  int textBase;			// baseline offset

  char *fontName;		// non-NULL if using a custom font
  XFontStruct *fontStruct;	// font info
  GC textGC;			// GC with text font
};

#endif
