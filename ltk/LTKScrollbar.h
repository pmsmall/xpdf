//========================================================================
//
// LTKScrollbar.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKSCROLLBAR_H
#define LTKSCROLLBAR_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include <gtypes.h>
#include <LTKWidget.h>

//------------------------------------------------------------------------
// LTKScrollbar
//------------------------------------------------------------------------

class LTKScrollbar: public LTKWidget {
public:

  //---------- constructor ----------

  LTKScrollbar(char *name1, int widgetNum1,
	       GBool vertical1, int minPos1, int maxPos1,
	       LTKIntValCbk moveCbk1);

  //---------- access ----------

  virtual long getEventMask();

  //---------- special access ----------

  void setLimits(int minPos1, int maxPos1);
  void setPos(int pos1, int size1);
  int getPos() { return pos; }
  void setScrollDelta(int scrollDelta1) { scrollDelta = scrollDelta1; }
  void setRepeatPeriod(int period) { repeatPeriod = period; }

  //---------- layout ----------

  virtual void layout1();

  //---------- drawing ----------

  virtual void redraw();

  //---------- callbacks and event handlers ----------

  virtual void buttonPress(int mx, int my, int button, GBool dblClick);
  virtual void buttonRelease(int mx, int my, int button, GBool click);
  virtual void mouseMove(int mx, int my, int pressedBtn);
  virtual void repeatEvent();

protected:

  void drawUpButton();
  void drawDownButton();
  void drawSlider(int pos1, Bool on);
  void doScroll();

  GBool vertical;		// orientation: 1=vertical, 0=horizontal
  int minPos, maxPos;		// min and max positions
				//   (minPos <= pos <= maxPos - size + 1)

  int pos, size;		// slider position and size
  int scrollDelta;		// scroll amount
  int repeatPeriod;		// time (us) between repeats
  int pixelPos, pixelSize;	// slider pos and size in pixels
  int pushPos;			// slider pos when pressed
  int pushPixel;		// mouse coord when pressed
  GBool sliderPressed;		// mouse has been pressed on slider
  GBool upPressed;		// up/left button is pressed
  GBool downPressed;		// down/right button is pressed

  LTKIntValCbk moveCbk;		// slider-move callback
};

#endif
