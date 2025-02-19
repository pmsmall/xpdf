//========================================================================
//
// LTKButtonDialog.cc
//
// Copyright 1997 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <stddef.h>
#include "LTKApp.h"
#include "LTKLabel.h"
#include "LTKButton.h"
#include "LTKEmpty.h"
#include "LTKBox.h"
#include "LTKWindow.h"
#include "LTKBorder.h"
#include "LTKButtonDialog.h"

//------------------------------------------------------------------------
// LTKButtonDialog
//------------------------------------------------------------------------

LTKButtonDialog::LTKButtonDialog(LTKWindow *overWin1, char *title,
				 char *line1, char *line2, char *line3,
				 char *trueBtnLabel, char *falseBtnLabel) {
  LTKBox *box;

  overWin = overWin1;
  widget = new LTKButtonDialogWidget(NULL, 0, line1, line2, line3,
				     trueBtnLabel, falseBtnLabel);
  box = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderNone, 0, 0, widget);
  win = new LTKWindow(overWin->getApp(), gTrue, title, NULL, NULL, box);
}

LTKButtonDialog::~LTKButtonDialog() {
  delete win;
}

GBool LTKButtonDialog::go() {
  win->layoutDialog(overWin, -1, -1);
  win->map();
  do {
    overWin->getApp()->doEvent(gTrue);
  } while (!widget->isDone());
  return widget->whichBtn();
}

//------------------------------------------------------------------------
// LTKButtonDialogWidget
//------------------------------------------------------------------------

LTKButtonDialogWidget::LTKButtonDialogWidget(char *name1, int widgetNum1,
					     char *line1, char *line2,
					     char *line3,
					     char *trueBtnLabel,
					     char *falseBtnLabel):
    LTKCompoundWidget(name1, widgetNum1) {
  LTKLabel *label1, *label2, *label3;
  LTKBox *labelBox1, *labelBox2, *labelBox3, *labelBox;
  LTKButton *trueBtn, *falseBtn;
  LTKEmpty *empty;
  LTKBox *trueBtnBox, *falseBtnBox, *emptyBox, *btnBox;

  labelBox1 = labelBox2 = labelBox3 = NULL;
  if (line1) {
    label1 = new LTKLabel(NULL, 0, ltkLabelStatic, -1, NULL, line1);
    labelBox1 = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderNone, 0, 0,
			   label1);
    if (line2) {
      label2 = new LTKLabel(NULL, 0, ltkLabelStatic, -1, NULL, line2);
      labelBox2 = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderNone, 0, 0,
			     label2);
      if (line3) {
	label3 = new LTKLabel(NULL, 0, ltkLabelStatic, -1, NULL, line3);
	labelBox3 = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderNone, 0, 0,
			       label3);
	labelBox = new LTKBox(NULL, 1, 3, 2, 2, 2, 2, ltkBorderNone, 0, 0,
			      labelBox1, labelBox2, labelBox3);
      } else {
	labelBox = new LTKBox(NULL, 1, 2, 2, 2, 2, 2, ltkBorderNone, 0, 0,
			      labelBox1, labelBox2);
      }
    } else {
      labelBox = new LTKBox(NULL, 1, 1, 2, 2, 2, 2, ltkBorderNone, 0, 0,
			    labelBox1);
    }
  } else {
    labelBox = new LTKBox(NULL, 1, 1, 2, 2, 2, 2, ltkBorderNone, 0, 0,
			  new LTKEmpty());
  }
  trueBtn = new LTKButton(NULL, 1, trueBtnLabel ? trueBtnLabel : "Ok",
			  ltkButtonClick, &buttonCbk);
  trueBtnBox = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderNone, 0, 0,
			  trueBtn);
  falseBtn = new LTKButton(NULL, 0, falseBtnLabel ? falseBtnLabel : "Cancel",
			   ltkButtonClick, &buttonCbk);
  falseBtnBox = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderNone, 0, 0,
			   falseBtn);
  empty = new LTKEmpty();
  emptyBox = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderNone, 1, 0,
			empty);
  btnBox = new LTKBox(NULL, 3, 1, 6, 6, 8, 6, ltkBorderNone, 1, 0,
		      trueBtnBox, emptyBox, falseBtnBox);
  box = new LTKBox(NULL, 1, 2, 0, 0, 0, 0, ltkBorderNone, 0, 0,
		   labelBox, btnBox);
  box->setCompoundParent(this);
  done = gFalse;
  btn = gFalse;
}

void LTKButtonDialogWidget::buttonCbk(LTKWidget *widget, int n, GBool on) {
  LTKButtonDialogWidget *w;

  w = (LTKButtonDialogWidget *)widget->getCompoundParent();
  w->done = gTrue;
  w->btn = n != 0;
}
