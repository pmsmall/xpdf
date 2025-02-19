//========================================================================
//
// LTKCompoundWidget.cc
//
// Compound widget base class.
//
// Copyright 1997 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "LTKApp.h"
#include "LTKWindow.h"
#include "LTKBox.h"
#include "LTKCompoundWidget.h"

//------------------------------------------------------------------------
// LTKCompoundWidget
//------------------------------------------------------------------------

LTKCompoundWidget::LTKCompoundWidget(char *nameA, int widgetNumA):
    LTKWidget(nameA, widgetNumA) {
  box = NULL;
}

LTKCompoundWidget::~LTKCompoundWidget() {
  delete box;
}

void LTKCompoundWidget::setParent(LTKWindow *parentA) {
  LTKWidget::setParent(parentA);
  box->setParent(parentA);
}

void LTKCompoundWidget::layout1() {
  box->layout1();
  width = box->getWidth();
  height = box->getHeight();
}

void LTKCompoundWidget::layout2(int xA, int yA, int widthA, int heightA) {
  box->layout2(xA, yA, widthA, heightA);
  LTKWidget::layout2(xA, yA, widthA, heightA);
}

void LTKCompoundWidget::layout3() {
  box->layout3();
}

void LTKCompoundWidget::map() {
  box->map();
}

void LTKCompoundWidget::redraw() {
  box->redraw();
}

void LTKCompoundWidget::redrawBackground() {
  box->redrawBackground();
}
