//========================================================================
//
// ltkbuild.widgets.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

ArgDesc windowArgs[] = {
  {"func",       argVal,     true,  "missingFunc"},
  {"title",      argVal,     true,  "missingTitle"},
  {"keyCbk",     argVal,     false, "NULL"},
  {NULL}
};

ArgDesc boxArgs[] = {
  {"name",       argVal,     false, "NULL"},
  {"x",          argVal,     true,  "missingX"},
  {"y",          argVal,     true,  "missingY"},
  {"left",       argVal,     false, "0"},
  {"right",      argVal,     false, "0"},
  {"top",        argVal,     false, "0"},
  {"bottom",     argVal,     false, "0"},
  {"flat",       argSel,     false, "ltkBorderNone"},
  {"raised",     argSel,     false, "ltkBorderRaised"},
  {"sunken",     argLastSel, false, "ltkBorderSunken"},
  {"xfill",      argVal,     false, "0"},
  {"yfill",      argVal,     false, "0"},
  {NULL}
};

ArgDesc box1Args[] = {
  {"name",       argVal,     false, "NULL"},
  {"x",          argVal,     false, "1"},
  {"y",          argVal,     false, "1"},
  {"left",       argVal,     false, "2"},
  {"right",      argVal,     false, "2"},
  {"top",        argVal,     false, "2"},
  {"bottom",     argVal,     false, "2"},
  {"flat",       argSel,     false, "ltkBorderNone"},
  {"raised",     argSel,     false, "ltkBorderRaised"},
  {"sunken",     argLastSel, false, "ltkBorderSunken"},
  {"xfill",      argVal,     false, "0"},
  {"yfill",      argVal,     false, "0"},
  {NULL}
};

#define boxArgX 1
#define boxArgY 2

ArgDesc buttonArgs[] = {
  {"name",       argVal,     false, "NULL"},
  {"label",      argVal,     true,  "missingLabel"},
  {"click",      argSel,     false, "ltkButtonClick"},
  {"sticky",     argSel,     false, "ltkButtonSticky"},
  {"toggle",     argLastSel, false, "ltkButtonToggle"},
  {"cbk",        argVal,     false, "NULL"},
  {"num",        argVal,     false, "0"},
  {NULL}
};

ArgDesc canvasArgs[] = {
  {"name",       argVal,     false, "NULL"},
  {"w",          argVal,     false, "32"},
  {"h",          argVal,     false, "32"},
  {"redraw",     argVal,     false, "NULL"},
  {"num",        argVal,     false, "0"},
  {NULL}
};

ArgDesc dblBufCanvasArgs[] = {
  {"name",       argVal,     false, "NULL"},
  {"w",          argVal,     false, "32"},
  {"h",          argVal,     false, "32"},
  {NULL}
};

ArgDesc emptyArgs[] = {
  {NULL}
};

ArgDesc iconButtonArgs[] = {
  {"name",       argVal,     false, "NULL"},
  {"bitmap",     argVal,     true,  "missingBitmap"},
  {"w",          argVal,     true,  "missingWidth"},
  {"h",          argVal,     true,  "missingHeight"},
  {"click",      argSel,     false, "ltkButtonClick"},
  {"sticky",     argSel,     false, "ltkButtonSticky"},
  {"toggle",     argLastSel, false, "ltkButtonToggle"},
  {"cbk",        argVal,     false, "NULL"},
  {"num",        argVal,     false, "0"},
  {NULL}
};

ArgDesc labelArgs[] = {
  {"name",       argVal,     false, "NULL"},
  {"length",     argVal,     false, "-1"},
  {"font",       argVal,     false, "NULL"},
  {"text",       argVal,     false, "NULL"},
  {NULL}
};

ArgDesc scrollbarArgs[] = {
  {"name",       argVal,     false, "NULL"},
  {"vert",       argSel,     true,  "true"},
  {"horiz",      argLastSel, true,  "false"},
  {"min",        argVal,     false, "0"},
  {"max",        argVal,     false, "0"},
  {"cbk",        argVal,     false, "NULL"},
  {"num",        argVal,     false, "0"},
  {NULL}
};

ArgDesc scrollingCanvasArgs[] = {
  {"name",       argVal,     false, "NULL"},
  {"w",          argVal,     false, "32"},
  {"h",          argVal,     false, "32"},
  {"mw",         argVal,     false, "32"},
  {"mh",         argVal,     false, "32"},
  {"layout",     argVal,     false, "NULL"},
  {NULL}
};

ArgDesc textInArgs[] = {
  {"name",       argVal,     false, "NULL"},
  {"length",     argVal,     true,  "missingLength"},
  {"font",       argVal,     false, "NULL"},
  {"cbk",        argVal,     false, "NULL"},
  {"num",        argVal,     false, "0"},
  {NULL}
};

BlockDesc windowTab[] = {
  {"Window",              "LTKWindow",              windowArgs},
  {NULL}
};

BlockDesc boxTab[] = {
  {"Box",                 "LTKBox",                 boxArgs},
  {"Box1",                "LTKBox",                 box1Args},
  {NULL}
};

BlockDesc widgetTab[] = {
  {"Box",                 "LTKBox",                 boxArgs},
  {"Box1",                "LTKBox",                 box1Args},
  {"Button",              "LTKButton",              buttonArgs},
  {"Canvas",              "LTKCanvas",              canvasArgs},
  {"DblBufCanvas",        "LTKDblBufCanvas",        dblBufCanvasArgs},
  {"Empty",               "LTKEmpty",               emptyArgs},
  {"IconButton",          "LTKButton",              iconButtonArgs},
  {"Label",               "LTKLabel",               labelArgs},
  {"Scrollbar",           "LTKScrollbar",           scrollbarArgs},
  {"ScrollingCanvas",     "LTKScrollingCanvas",     scrollingCanvasArgs},
  {"TextIn",              "LTKTextIn",              textInArgs},
  {NULL}
};
