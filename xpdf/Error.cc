//========================================================================
//
// Error.cc
//
//========================================================================

#pragma implementation

#include <stdio.h>
#include <stdarg.h>
#include <stypes.h>
#include "Flags.h"
#include "Error.h"

// Send error messages to /dev/tty instead of stderr.
Boolean errorsToTTY;

// File to send error (and other) messages to.
FILE *errFile;

void errorInit() {
  if (!errorsToTTY || !(errFile = fopen("/dev/tty", "w")))
    errFile = stderr;
}

void error(int pos, char *msg, ...) {
  va_list args;

  fprintf(errFile, "Error (%d): ", pos);
  va_start(args, msg);
  vfprintf(errFile, msg, args);
  va_end(args);
  fprintf(errFile, "\n");
  fflush(errFile);
}
