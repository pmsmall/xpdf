//========================================================================
//
// LTKScrollbar.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKSCROLLBAR_H
#define LTKSCROLLBAR_H

#pragma interface

#include <X11/Xlib.h>
#include <stypes.h>
#include <LTKWidget.h>

class LTKScrollbar;
typedef void (*LTKScrollbarCbk)(LTKScrollbar *scrollbar, int widgetNum,
				int value);

class LTKScrollbar: public LTKWidget {
public:

  LTKScrollbar(char *name1, Boolean vertical1, int minPos1, int maxPos1,
	       LTKScrollbarCbk moveCbk1, int widgetNum1);

  virtual LTKWidget *copy() { return new LTKScrollbar(this); }

  virtual long getEventMask();

  virtual void layout1();

  virtual void redraw();

  void setLimits(int minPos1, int maxPos1);
  void setPos(int pos1, int size1);
  int getPos() { return pos; }
  void setScrollDelta(int scrollDelta1) { scrollDelta = scrollDelta1; }

  virtual void buttonPress(int mx, int my, int button);
  virtual void buttonRelease(int mx, int my, int button);
  virtual void mouseMove(int mx, int my);
  virtual void repeatEvent();

protected:

  LTKScrollbar(LTKScrollbar *scrollbar);
  void drawUpButton();
  void drawDownButton();
  void drawSlider(int pos1, Bool on);
  void doScroll();

  Boolean vertical;		// orientation: 1=vertical, 0=horizontal
  int minPos, maxPos;		// min and max positions
				//   (minPos <= pos <= maxPos - size + 1)
  LTKScrollbarCbk moveCbk;	// slider-move callback
  int widgetNum;		// widget number (for callback)

  int pos, size;		// slider position and size
  int scrollDelta;		// scroll amount
  int pixelPos, pixelSize;	// slider pos and size in pixels
  int pushPos;			// slider pos when pressed
  int pushPixel;		// mouse coord when pressed
  Boolean sliderPressed;	// mouse has been pressed on slider
  Boolean upPressed;		// up/left button is pressed
  Boolean downPressed;		// down/right button is pressed
};

#endif
