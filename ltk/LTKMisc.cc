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
#include <unistd.h>
#include <stdarg.h>
#ifndef VMS
#include <pwd.h>
#endif
#include <LTKMisc.h>
#include <LTKWindow.h>
#include <LTKWidget.h>

#ifndef VMS
GString *ltkGetHomeDir() {
  char *s;
  struct passwd *pw;
  GString *ret;

  if ((s = getenv("HOME"))) {
    ret = new GString(s);
  } else {
    if ((s = getenv("USER")))
      pw = getpwnam(s);
    else
      pw = getpwuid(getuid());
    if (pw)
      ret = new GString(pw->pw_dir);
    else
      ret = new GString(".");
  }
  return ret;
}
#endif

void ltkError(char *msg, ...) {
  va_list args;

  fprintf(stderr, "LTK Error: ");
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, "\n");
}
