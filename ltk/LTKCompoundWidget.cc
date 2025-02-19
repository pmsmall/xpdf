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

LTKCompoundWidget::LTKCompoundWidget(char *name1, int widgetNum1):
    LTKWidget(name1, widgetNum1) {
  box = NULL;
}

LTKCompoundWidget::~LTKCompoundWidget() {
  delete box;
}

void LTKCompoundWidget::setParent(LTKWindow *parent1) {
  LTKWidget::setParent(parent1);
  box->setParent(parent1);
}

void LTKCompoundWidget::layout1() {
  box->layout1();
  width = box->getWidth();
  height = box->getHeight();
}

void LTKCompoundWidget::layout2(int x1, int y1, int width1, int height1) {
  box->layout2(x1, y1, width1, height1);
  LTKWidget::layout2(x1, y1, width1, height1);
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
