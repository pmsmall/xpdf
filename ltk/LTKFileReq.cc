//========================================================================
//
// LTKFileReq.cc
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
#include "GString.h"
#include "gfile.h"
#include "LTKBox.h"
#include "LTKLabel.h"
#include "LTKList.h"
#include "LTKScrollbar.h"
#include "LTKTextIn.h"
#include "LTKEmpty.h"
#include "LTKFileReq.h"

#define dirLabelLen 50

LTKFileReq::LTKFileReq(char *name1, int widgetNum1,
		       LTKStringValCbk selectCbk1, char *fontName1):
    LTKCompoundWidget(name1, widgetNum1) {
  selectCbk = selectCbk1;
  fontName = fontName1;
  dir = getCurrentDir();
  selection = NULL;
  makeWidgets();
}

void LTKFileReq::makeWidgets() {
  LTKEmpty *empty;
  LTKLabel *dirLabel, *fileLabel;
  LTKBox *dirNameBox1, *dirNameBox2, *dirNameBox;
  LTKBox *fileNameBox1, *fileNameBox2, *fileNameBox;
  LTKBox *listBox, *hScrollBox, *vScrollBox, *emptyBox, *listBox2;

  dirLabel = new LTKLabel(NULL, 0, ltkLabelStatic, -1, NULL, "Dir:");
  dirNameBox1 = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderNone, 0, 0,
			   dirLabel);
  dirName = new LTKTextIn(NULL, 0, 40, fontName, &dirNameCbk, NULL);
  dirNameBox2 = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderSunken, 1, 0,
			   dirName);
  dirNameBox = new LTKBox(NULL, 2, 1, 0, 0, 0, 2, ltkBorderNone, 1, 0,
			  dirNameBox1, dirNameBox2);

  list = new LTKList(NULL, 0, 250, 10, gTrue, fontName);
  list->setClickCbk(&clickCbk);
  list->setDblClickCbk(&dblClickCbk);
  listBox = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderSunken, 1, 1,
		       list);
  hScrollbar = new LTKScrollbar(NULL, 0, gFalse, 0, 1, &hScrollCbk);
  hScrollBox = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderNone, 1, 0,
			 hScrollbar);
  vScrollbar = new LTKScrollbar(NULL, 0, gTrue, 0, 1, &vScrollCbk);
  vScrollBox = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderNone, 0, 1,
			  vScrollbar);
  empty = new LTKEmpty();
  emptyBox = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderNone, 0, 0,
			empty);
  listBox2 = new LTKBox(NULL, 2, 2, 0, 0, 0, 2, ltkBorderNone, 1, 1,
			listBox, vScrollBox, hScrollBox, emptyBox);

  fileLabel = new LTKLabel(NULL, 0, ltkLabelStatic, -1, NULL, "File:");
  fileNameBox1 = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderNone, 0, 0,
			    fileLabel);
  fileName = new LTKTextIn(NULL, 0, 40, fontName, NULL, NULL);
  fileNameBox2 = new LTKBox(NULL, 1, 1, 0, 0, 0, 0, ltkBorderSunken, 1, 0,
			    fileName);
  fileNameBox = new LTKBox(NULL, 2, 1, 0, 0, 0, 0, ltkBorderNone, 1, 0,
			   fileNameBox1, fileNameBox2);

  box = new LTKBox(NULL, 1, 3, 0, 0, 0, 0, ltkBorderNone, 1, 1,
		   dirNameBox, listBox2, fileNameBox);
  box->setCompoundParent(this);
}

LTKFileReq::~LTKFileReq() {
  delete dir;
  if (selection)
    delete selection;
}

GString *LTKFileReq::getSelection() {
  if (selection) {
    delete selection;
    selection = NULL;
  }
  if (fileName->getText()->getLength() == 0)
    return NULL;
  selection = dir->copy();
  appendToPath(selection, fileName->getText()->getCString());
  return selection;
}

void LTKFileReq::setDir(GString *dir1) {
  delete dir;
  dir = dir1->copy();
  if (mapped)
    loadDirList();
}

void LTKFileReq::layout3() {
  LTKCompoundWidget::layout3();
  hScrollbar->setLimits(0, list->getMaxWidth() - 1);
  hScrollbar->setPos(hScrollbar->getPos(), list->getWidth());
  vScrollbar->setLimits(0, list->getNumLines() - 1);
  vScrollbar->setPos(vScrollbar->getPos(), list->getDisplayedLines());
}

void LTKFileReq::map() {
  loadDirList();
  LTKCompoundWidget::map();
}

void LTKFileReq::loadDirList() {
  GDir *d;
  GDirEntry *ent;
  GString *s;
  int i;

  // initialize
  makePathAbsolute(dir);
  dirName->setText(dir->getCString());
  list->deleteAll();

  // scan directory and add entries to list
  d = new GDir(dir->getCString());
  numDirs = 0;
  while ((ent = d->getNextEntry())) {
    if (ent->isDir()) {
      s = ent->getName()->copy();
      for (i = 0; i < numDirs; ++i) {
	if (s->cmpN(list->getLine(i), list->getLine(i)->getLength() - 1) < 0)
	  break;
      }
#ifndef VMS
      s->append('/');
#endif
      list->insertLine(i, s->getCString());
      delete s;
      ++numDirs;
    } else {
      s = ent->getName();
      for (i = numDirs; i < list->getNumLines(); ++i) {
	if (s->cmp(list->getLine(i)) < 0)
	  break;
      }
      list->insertLine(i, s->getCString());
    }
    delete ent;
  }
  delete d;

  // reset scrollbars
  hScrollbar->setLimits(0, list->getMaxWidth() - 1);
  hScrollbar->setPos(0, list->getWidth());
  vScrollbar->setLimits(0, list->getNumLines() - 1);
  vScrollbar->setPos(0, list->getDisplayedLines());

}

void LTKFileReq::clickCbk(LTKWidget *widget, int widgetNum, int line) {
  LTKFileReq *fileReq;

  fileReq = (LTKFileReq *)widget->getCompoundParent();
  if (line >= fileReq->numDirs)
    fileReq->fileName->setText(fileReq->list->getLine(line)->getCString());
  else
    fileReq->fileName->setText("");
}

void LTKFileReq::dblClickCbk(LTKWidget *widget, int widgetNum, int line) {
  LTKFileReq *fileReq;
  GString *subDir;

  fileReq = (LTKFileReq *)widget->getCompoundParent();
  if (line < fileReq->numDirs) {
    subDir = fileReq->list->getLine(line);
#ifndef VMS
    subDir->del(subDir->getLength() - 1);
#endif
    appendToPath(fileReq->dir, subDir->getCString());
    fileReq->loadDirList();
  } else {
    if (fileReq->selectCbk)
      (*fileReq->selectCbk)(widget, widgetNum, fileReq->getSelection());
  }
}

void LTKFileReq::dirNameCbk(LTKWidget *widget, int widgetNum, GString *val) {
  LTKFileReq *fileReq;

  fileReq = (LTKFileReq *)widget->getCompoundParent();
  delete fileReq->dir;
  fileReq->dir = val->copy();
  fileReq->loadDirList();
}

void LTKFileReq::hScrollCbk(LTKWidget *widget, int widgetNum, int val) {
  LTKFileReq *fileReq;

  fileReq = (LTKFileReq *)widget->getCompoundParent();
  fileReq->list->scrollTo(fileReq->vScrollbar->getPos(), val);
  XSync(widget->getDisplay(), False);
}

void LTKFileReq::vScrollCbk(LTKWidget *widget, int widgetNum, int val) {
  LTKFileReq *fileReq;

  fileReq = (LTKFileReq *)widget->getCompoundParent();
  fileReq->list->scrollTo(val, fileReq->hScrollbar->getPos());
  XSync(widget->getDisplay(), False);
}
