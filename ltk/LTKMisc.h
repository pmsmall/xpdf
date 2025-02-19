//========================================================================
//
// LTKMisc.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef LTKMISC_H
#define LTKMISC_H

#ifdef __GNUC__
#pragma interface
#endif

#include <GString.h>

#ifndef VMS
extern GString *ltkGetHomeDir(void);
#endif

extern void ltkError(char *msg, ...);

#endif
