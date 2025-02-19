//========================================================================
//
// LTKMisc.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include "LTKMisc.h"
#include "LTKWindow.h"
#include "LTKWidget.h"

void ltkError(char *msg, ...) {
  va_list args;

  fprintf(stderr, "LTK Error: ");
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, "\n");
}
