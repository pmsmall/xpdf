//========================================================================
//
// LTKButton.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#pragma implementation

#include <stdlib.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <LTKWindow.h>
#include <LTKButton.h>
#include <LTKBorder.h>

LTKButton::LTKButton(char *name1, char *label1, LTKButtonAction action1,
		     LTKButtonCbk pressCbk1, int widgetNum1):
    LTKWidget(ltkButton, name1) {
  label = new String(label1);
  icon = None;
  iconData = NULL;
  action = action1;
  pressCbk = pressCbk1;
  widgetNum = widgetNum1;
  on = false;
}

LTKButton::LTKButton(char *name1, unsigned char *iconData1,
		     int iconWidth1, int iconHeight1, LTKButtonAction action1,
		     LTKButtonCbk pressCbk1, int widgetNum1):
    LTKWidget(ltkButton, name1) {
  label = NULL;
  icon = None;
  iconData = iconData1;
  iconWidth = iconWidth1;
  iconHeight = iconHeight1;
  action = action1;
  pressCbk = pressCbk1;
  widgetNum = widgetNum1;
  on = false;
}

LTKButton::LTKButton(LTKButton *button):
    LTKWidget(button) {
  label = button->label ? button->label->copy() : (String *)NULL;
  iconData = button->iconData;
  iconWidth = button->iconWidth;
  iconHeight = button->iconHeight;
  icon = None;
  action = button->action;
  pressCbk = button->pressCbk;
  widgetNum = button->widgetNum;
  on = false;
}

LTKButton::~LTKButton() {
  if (label)
    delete label;
  if (icon)
    XFreePixmap(getDisplay(), icon);
}

long LTKButton::getEventMask() {
  return ExposureMask | ButtonPressMask | ButtonReleaseMask;
}

void LTKButton::layout1() {
  XFontStruct *fontStruct;
  XCharStruct extents;
  int direction, ascent, descent;

  if (label) {
    fontStruct = getXFontStruct();
    XTextExtents(fontStruct, label->getCString(), label->getLength(),
		 &direction, &ascent, &descent, &extents);
    textWidth = extents.width;
    textHeight = fontStruct->ascent + fontStruct->descent;
    textBase = fontStruct->ascent;
    width = textWidth + 12 + 2 * ltkBorderWidth;
    height = textHeight + 4 + 2 * ltkBorderWidth;
  } else {
    width = iconWidth + 12 + 2 * ltkBorderWidth;
    height = iconHeight + 4 + 2 * ltkBorderWidth;
  }
}

void LTKButton::layout3() {
  LTKWidget::layout3();
  if (iconData && icon == None) {
    icon = XCreatePixmapFromBitmapData(
             getDisplay(), getXWindow(), (char *)iconData,
             iconWidth, iconHeight, getFgColor(), getBgColor(),
	     DefaultDepth(getDisplay(), getScreenNum()));
  }
}

void LTKButton::redraw() {
  int tx, ty;

  ltkDrawBorder(getDisplay(), xwin, getBrightGC(), getDarkGC(), getBgGC(),
		0, 0, width, height, on ? ltkBorderSunken : ltkBorderRaised);
  if (label) {
    tx = (width - textWidth) / 2;
    ty = (height - textHeight) / 2 + textBase;
    XDrawString(getDisplay(), xwin, getFgGC(), tx, ty,
		label->getCString(), label->getLength());
  } else {
    XCopyArea(getDisplay(), icon, getXWindow(), getFgGC(),
	      0, 0, iconWidth, iconHeight,
	      (width - iconWidth) / 2, (height - iconHeight) / 2);
  }
}

void LTKButton::buttonPress(int mx, int my, int button) {
  switch (action) {
  case ltkButtonClick:
    setState(true);
    break;
  case ltkButtonSticky:
    setState(true);
    break;
  case ltkButtonToggle:
    setState(!on);
    break;
  }
}

void LTKButton::buttonRelease(int mx, int my, int button) {
  switch (action) {
  case ltkButtonClick:
    setState(false);
    break;
  case ltkButtonSticky:
    break;
  case ltkButtonToggle:
    break;
  }
  if (pressCbk)
    (*pressCbk)(this, widgetNum, on);
}

void LTKButton::setState(Boolean on1) {
  if (on1 != on) {
    on = on1;
    ltkDrawBorder(getDisplay(), xwin, getBrightGC(), getDarkGC(), getBgGC(),
		  0, 0, width, height, on ? ltkBorderSunken : ltkBorderRaised);
  }
}
