//========================================================================
//
// Error.h
//
//========================================================================

#ifndef ERROR_H
#define ERROR_H

#pragma interface

// File to send error (and other) messages to.
extern FILE *errFile;

extern void errorInit();

extern void error(int pos, char *msg, ...);

#endif
