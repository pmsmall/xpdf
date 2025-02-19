//========================================================================
//
// LTKMisc.cc
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#pragma implementation

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <pwd.h>
#include <LTKMisc.h>
#include <LTKWindow.h>
#include <LTKWidget.h>

String *ltkGetHomeDir() {
  char *s;
  struct passwd *pw;
  String *ret;

  if ((s = getenv("HOME"))) {
    ret = new String(s);
  } else {
    if ((s = getenv("USER")))
      pw = getpwnam(s);
    else
      pw = getpwuid(getuid());
    if (pw)
      ret = new String(pw->pw_dir);
    else
      ret = new String(".");
  }
  return ret;
}

void ltkError(char *msg, ...) {
  va_list args;

  fprintf(stderr, "LTK Error: ");
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, "\n");
}
