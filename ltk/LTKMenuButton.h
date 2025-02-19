//========================================================================
//
// LTKMenuButton.h
//
// Copyright 1999 Derek B. Noonburg
//
//========================================================================

#ifndef LTKMENUBUTTON_H
#define LTKMENUBUTTON_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include "gtypes.h"
#include "GString.h"
#include "LTKWidget.h"
#include "LTKMenu.h"

//------------------------------------------------------------------------
// LTKMenuButton
//------------------------------------------------------------------------

class LTKMenuButton: public LTKWidget {
public:

  //---------- constructors and destructor ----------

  LTKMenuButton(char *nameA, int widgetNumA, LTKMenu *menAu);

  virtual ~LTKMenuButton();

  //---------- access ----------

  virtual long getEventMask();
  void setInitialMenuItem(LTKMenuItem *item) { menuItem = item; }

  //---------- layout ----------

  virtual void layout1();

  //---------- drawing ----------

  virtual void redraw();

  //---------- callbacks and event handlers ----------

  virtual void buttonPress(int bx, int by, int button, GBool dblClick);

  //---------- used by callbacks ----------

  void setMenuItem(LTKMenuItem *item);

protected:

  LTKMenu *menu;		// associated menu
  LTKMenuItem *menuItem;	// current item
  LTKMenuCbk *cbks;		// original menu item callbacks
  int textWidth, textHeight;	// size of text
  int textBase;			// baseline offset
};

#endif
