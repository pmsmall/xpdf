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
#define xpdfVersion "0.7"

// supported PDF version
#define pdfVersion "1.2"
#define pdfVersionNum 1.2

// copyright notice
#define xpdfCopyright "Copyright \251 1996, 1997 Derek B. Noonburg"

// paper size (in points) for PostScript output
// (set to American 8.5"x11" for now; will be configurable later)
#define paperWidth  612
#define paperHeight 792

// config file name
#if defined(VMS) || defined(__EMX__)
#define xpdfConfigFile "xpdfrc"
#else
#define xpdfConfigFile ".xpdfrc"
#endif

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
#define uncompressCmd "gzip -d -q"
#else
#define uncompressCmd "uncompress"
#endif // USE_GZIP
#else // NO_POPEN
// command to uncompress to stdout
#ifdef USE_GZIP
#define uncompressCmd "gzip -d -c -q"
#else
#ifdef __EMX__
#define uncompressCmd "compress -d -c"
#else
#define uncompressCmd "uncompress -c"
#endif // __EMX__
#endif // USE_GZIP
#endif // NO_POPEN

#endif
