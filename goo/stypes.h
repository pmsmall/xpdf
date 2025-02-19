/*
 * stypes.h
 *
 * Some useful simple types.
 */

#ifndef STYPES_H
#define STYPES_H

typedef int Boolean;
#define true 1
#define false 0

#if defined(_AIX) || defined(__osf__)
#include <sys/types.h>
#else
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
#endif

#endif
