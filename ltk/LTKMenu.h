//========================================================================
//
// LTKMenu.h
//
// Menus and menu items.
//
// Copyright 1997 Derek B. Noonburg.
//
//========================================================================

#ifndef LTKMENU_H
#define LTKMENU_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stddef.h>
#include <X11/Xlib.h>
#include "gtypes.h"
#include "GString.h"

class LTKWindow;
class LTKWidget;
class LTKMenuItem;

//------------------------------------------------------------------------
// callback types
//------------------------------------------------------------------------

// Menu item selection.
typedef void (*LTKMenuCbk)(LTKMenuItem *item);

//------------------------------------------------------------------------
// LTKMenu
//------------------------------------------------------------------------

class LTKMenu {
public:

  //---------- constructor and destructor ----------

  // Constructor.  The extra args are the list of menu items.
  LTKMenu(char *title1, int numItems1, ...);

  // Destructor.
  ~LTKMenu();

  //---------- access ----------

  Window getXWindow() { return xwin; }
  int getNumItems() { return numItems; }
  LTKMenuItem *getItem(int i) { return items[i]; }
  void setParentWidget(LTKWidget *widget) { this->widget = widget; }
  LTKWidget *getParentWidget() { return widget; }

  //---------- drawing ----------

  // Post the menu.  Put the top left corner as close as possible
  // to <x1>,<y1>.
  void post(LTKWindow *win1, int x1, int y1, LTKMenu *parent1);

  //---------- event handlers ----------

  void redraw();
  void buttonPress(int mx, int my, int button, GBool dblClick);
  void buttonRelease(int mx, int my, int button, GBool click);
  void mouseMove(int mx, int my, int btn);

private:

  void repost();
  void unpost();
  void done();

  char *title;			// menu title (may be NULL)
  LTKMenuItem **items;		// array of items
  int numItems;			// number of items

  LTKWindow *win;		// parent window
  LTKMenu *parent;		// parent menu, or NULL if none
  int x, y;			// current position
  int width, height;		// size of window
  int itemHeight;		// height of menu item
  Display *display;		// X display
  Window xwin;			// X window
  GC fgGC, bgGC;		// foreground/background GCs
  GC brightGC, darkGC;		// shaded border GCs

  int currentItem;		// currently selected item or -1 if none
  int currentY;			// y coordinate of current item
  LTKMenu *currentSubmenu;	// currently posted submenu

  LTKWidget *widget;		// parent widget (if any)
};

//------------------------------------------------------------------------
// LTKMenuItem
//------------------------------------------------------------------------

class LTKMenuItem {
public:

  //---------- constructor ----------

  // Constructor.
  LTKMenuItem(char *text1, char *shortcut1, int itemNum1,
	      LTKMenuCbk cbk1, LTKMenu *submenu1);

  //---------- access ----------

  int getItemNum() { return itemNum; }
  char *getText() { return text; }
  LTKMenuCbk getCbk() { return cbk; }
  void setCbk(LTKMenuCbk cbk) { this->cbk = cbk; }
  LTKMenu *getParent() { return parent; }
  void setParent(LTKMenu *parent) { this->parent = parent; }

private:

  int getWidth();
  int getHeight();

  char *text;			// text of the menu item
  char *shortcut;		// shortcut for menu item
  int itemNum;			// item number
  LTKMenuCbk cbk;		// selection callback
  LTKMenu *submenu;		// pointer to submenu, or NULL if none
  LTKMenu *parent;		// parent menu

  friend class LTKMenu;
};

#endif
