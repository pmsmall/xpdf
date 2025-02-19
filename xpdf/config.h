//========================================================================
//
// config.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef CONFIG_H
#define CONFIG_H

//------------------------------------------------------------------------
// general constants
//------------------------------------------------------------------------

// xpdf version
#define xpdfVersion "0.3"

// supported PDF version
#define pdfVersion "1.1"
#define pdfVersionNum 1.1

// copyright notice
#define xpdfCopyright "Copyright \251 1996 Derek B. Noonburg"

//------------------------------------------------------------------------
// X-related constants
//------------------------------------------------------------------------

// maximum size of color cube to allocate
#define maxColorCube 5

// number of fonts to cache
#define fontCacheSize 16

//------------------------------------------------------------------------
// misc
//------------------------------------------------------------------------

// command to uncompress to stdout
#define uncompressCmd "uncompress -c"

#endif
