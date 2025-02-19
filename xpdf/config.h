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
#define xpdfVersion "0.7a"

// supported PDF version
#define pdfVersion "1.2"
#define pdfVersionNum 1.2

// copyright notice
#define xpdfCopyright "Copyright \251 1996-1998 Derek B. Noonburg"

// paper size (in points) for PostScript output
// (eventually this will be configurable from the print dialog)
#ifdef A4_PAPER
#define paperWidth  595    // ISO A4 (210x297 mm)
#define paperHeight 842
#else
#define paperWidth  612    // American letter (8.5x11")
#define paperHeight 792
#endif

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
// fopen modes for binary files
//------------------------------------------------------------------------

#ifdef __WIN32__
#define FOPEN_READ_BIN "rb"
#define FOPEN_WRITE_BIN "wb"
#else
#define FOPEN_READ_BIN "r"
#define FOPEN_WRITE_BIN "w"
#endif

//------------------------------------------------------------------------
// uncompress program
//------------------------------------------------------------------------

#ifdef HAVE_POPEN

// command to uncompress to stdout
#  ifdef USE_GZIP
#    define uncompressCmd "gzip -d -c -q"
#  else
#    ifdef __EMX__
#      define uncompressCmd "compress -d -c"
#    else
#      define uncompressCmd "uncompress -c"
#    endif // __EMX__
#  endif // USE_GZIP

#else // HAVE_POPEN

// command to uncompress a file
#  ifdef USE_GZIP
#    define uncompressCmd "gzip -d -q"
#  else
#    define uncompressCmd "uncompress"
#  endif // USE_GZIP

#endif // HAVE_POPEN

#endif
