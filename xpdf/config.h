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
#define xpdfVersion "0.5"

// supported PDF version
#define pdfVersion "1.1"
#define pdfVersionNum 1.1

// copyright notice
#define xpdfCopyright "Copyright \251 1996 Derek B. Noonburg"

// paper size (in points) for PostScript output
// (set to American 8.5"x11" for now; will be configurable later)
#define paperWidth  612
#define paperHeight 792

//------------------------------------------------------------------------
// X-related constants
//------------------------------------------------------------------------

// default maximum size of color cube to allocate
#define defaultRGBCube 5

// number of fonts to cache
#define fontCacheSize 16

//------------------------------------------------------------------------
// misc
//------------------------------------------------------------------------

#ifdef NO_POPEN
// command to uncompress a file
#ifdef USE_GZIP
#define uncompressCmd "gzip -d"
#else
#define uncompressCmd "uncompress"
#endif
#else // NO_POPEN
// command to uncompress to stdout
#ifdef USE_GZIP
#define uncompressCmd "gzip -d -c"
#else
#define uncompressCmd "uncompress -c"
#endif
#endif // NO_POPEN

#endif
