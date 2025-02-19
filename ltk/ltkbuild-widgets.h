//========================================================================
//
// ltkbuild-widgets.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

ArgDesc windowArgs[] = {
  {"func",           argVal,     gTrue,  "missingFunc"},
  {"dialog",         argVal,     gFalse, "gFalse"},
  {"title",          argVal,     gTrue,  "missingTitle"},
  {NULL}
};

ArgDesc boxArgs[] = {
  {"name",           argVal,     gFalse, "NULL"},
  {"x",              argVal,     gTrue,  "missingX"},
  {"y",              argVal,     gTrue,  "missingY"},
  {"left",           argVal,     gFalse, "0"},
  {"right",          argVal,     gFalse, "0"},
  {"top",            argVal,     gFalse, "0"},
  {"bottom",         argVal,     gFalse, "0"},
  {"flat",           argSel,     gFalse, "ltkBorderNone"},
  {"raised",         argSel,     gFalse, "ltkBorderRaised"},
  {"sunken",         argLastSel, gFalse, "ltkBorderSunken"},
  {"xfill",          argVal,     gFalse, "0"},
  {"yfill",          argVal,     gFalse, "0"},
  {NULL}
};

ArgDesc box1Args[] = {
  {"name",           argVal,     gFalse, "NULL"},
  {"x",              argVal,     gFalse, "1"},
  {"y",              argVal,     gFalse, "1"},
  {"left",           argVal,     gFalse, "2"},
  {"right",          argVal,     gFalse, "2"},
  {"top",            argVal,     gFalse, "2"},
  {"bottom",         argVal,     gFalse, "2"},
  {"flat",           argSel,     gFalse, "ltkBorderNone"},
  {"raised",         argSel,     gFalse, "ltkBorderRaised"},
  {"sunken",         argLastSel, gFalse, "ltkBorderSunken"},
  {"xfill",          argVal,     gFalse, "0"},
  {"yfill",          argVal,     gFalse, "0"},
  {NULL}
};

#define boxArgX 1
#define boxArgY 2

ArgDesc buttonArgs[] = {
  {"name",           argVal,     gFalse, "NULL"},
  {"num",            argVal,     gFalse, "0"},
  {"label",          argVal,     gTrue,  "missingLabel"},
  {"click",          argSel,     gFalse, "ltkButtonClick"},
  {"sticky",         argSel,     gFalse, "ltkButtonSticky"},
  {"toggle",         argLastSel, gFalse, "ltkButtonToggle"},
  {"press",          argVal,     gFalse, "NULL"},
  {NULL}
};

ArgDesc iconButtonArgs[] = {
  {"name",           argVal,     gFalse, "NULL"},
  {"num",            argVal,     gFalse, "0"},
  {"bitmap",         argVal,     gTrue,  "missingBitmap"},
  {"w",              argVal,     gTrue,  "missingWidth"},
  {"h",              argVal,     gTrue,  "missingHeight"},
  {"click",          argSel,     gFalse, "ltkButtonClick"},
  {"sticky",         argSel,     gFalse, "ltkButtonSticky"},
  {"toggle",         argLastSel, gFalse, "ltkButtonToggle"},
  {"press",          argVal,     gFalse, "NULL"},
  {NULL}
};

ArgDesc canvasArgs[] = {
  {"name",           argVal,     gFalse, "NULL"},
  {"num",            argVal,     gFalse, "0"},
  {"w",              argVal,     gFalse, "32"},
  {"h",              argVal,     gFalse, "32"},
  {"redraw",         argVal,     gFalse, "NULL"},
  {NULL}
};

ArgDesc dblBufCanvasArgs[] = {
  {"name",           argVal,     gFalse, "NULL"},
  {"num",            argVal,     gFalse, "0"},
  {"w",              argVal,     gFalse, "32"},
  {"h",              argVal,     gFalse, "32"},
  {NULL}
};

ArgDesc emptyArgs[] = {
  {NULL}
};

ArgDesc labelArgs[] = {
  {"name",           argVal,     gFalse, "NULL"},
  {"num",            argVal,     gFalse, "0"},
  {"length",         argVal,     gFalse, "-1"},
  {"font",           argVal,     gFalse, "NULL"},
  {"text",           argVal,     gFalse, "NULL"},
  {NULL}
};

ArgDesc scrollbarArgs[] = {
  {"name",           argVal,     gFalse, "NULL"},
  {"num",            argVal,     gFalse, "0"},
  {"vert",           argSel,     gTrue,  "gTrue"},
  {"horiz",          argLastSel, gTrue,  "gFalse"},
  {"min",            argVal,     gFalse, "0"},
  {"max",            argVal,     gFalse, "0"},
  {"move",           argVal,     gFalse, "NULL"},
  {NULL}
};

ArgDesc scrollingCanvasArgs[] = {
  {"name",           argVal,     gFalse, "NULL"},
  {"num",            argVal,     gFalse, "0"},
  {"w",              argVal,     gFalse, "32"},
  {"h",              argVal,     gFalse, "32"},
  {"mw",             argVal,     gFalse, "32"},
  {"mh",             argVal,     gFalse, "32"},
  {NULL}
};

ArgDesc textInArgs[] = {
  {"name",           argVal,     gFalse, "NULL"},
  {"num",            argVal,     gFalse, "0"},
  {"mw",             argVal,     gFalse, "8"},
  {"font",           argVal,     gFalse, "NULL"},
  {"done",           argVal,     gFalse, "NULL"},
  {"tab",            argVal,     gFalse, "NULL"},
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
