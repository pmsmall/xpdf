//========================================================================
//
// fileNames.h
//
// Miscellaneous file and directory name manipulation.
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef FILENAMES_H
#define FILENAMES_H

#include <gtypes.h>

class GString;

// Get home directory path.
extern GString *getHomeDir();

// Append a file name to a path string.  <path> may be an empty
// string, denoting the current directory).
extern GString *appendToPath(GString *path, char *fileName);

// Grab the path from the front of the file name.  If there is no
// directory component in <fileName>, returns an empty string.
extern GString *grabPath(char *fileName);

// Is this an absolute path or file name?
extern GBool isAbsolutePath(char *path);

#endif
