//========================================================================
//
// LTKButton.h
//
//========================================================================

#ifndef LTKBUTTON_H
#define LTKBUTTON_H

#pragma interface

#include <X11/Xlib.h>
#include <stypes.h>
#include <String.h>
#include <LTKWidget.h>

class LTKButton;
typedef void (*LTKButtonCbk)(LTKButton *button, int widgetNum, Boolean on);

typedef enum {
  ltkButtonClick,		// momentarily on
  ltkButtonSticky,		// stays on until setState() is called
  ltkButtonToggle		// press on, press off
} LTKButtonAction;

class LTKButton: public LTKWidget {
public:

  LTKButton(char *name1, char *label1, LTKButtonAction action1,
	    LTKButtonCbk pressCbk1, int widgetNum1);

  LTKButton(char *name1, unsigned char *iconData1,
	    int iconWidth1, int iconHeight1, LTKButtonAction action1,
	    LTKButtonCbk pressCbk1, int widgetNum1);

  ~LTKButton();

  virtual LTKWidget *copy() { return new LTKButton(this); }

  virtual long getEventMask();

  virtual void layout1();

  virtual void layout3();

  virtual void redraw();

  virtual void buttonPress(int bx, int by, int button);
  virtual void buttonRelease(int bx, int by, int button);

  void setState(Boolean on1);

protected:

  LTKButton(LTKButton *button);

  String *label;		// label (button has either label or icon)
  Pixmap icon;			// icon
  unsigned char *iconData;	// the icon data (bitmap format)
  int iconWidth, iconHeight;	// the icon size
  LTKButtonAction action;	// action kind
  LTKButtonCbk pressCbk;	// button-press callback
  int widgetNum;		// widget number (for callback)
  Boolean on;			// current state
  int textWidth, textHeight;	// size of text
  int textBase;			// baseline offset
};

#endif
