//========================================================================
//
// LTKMisc.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKMISC_H
#define LTKMISC_H

#pragma interface

#include <String.h>

extern String *ltkGetHomeDir(void);

extern void ltkError(char *msg, ...);

#endif
