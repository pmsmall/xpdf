//========================================================================
//
// fileNames.cc
//
// Miscellaneous file and directory name manipulation.
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

//~ The VMS code here needs work...

#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#ifndef VMS
#include <pwd.h>
#endif
#include <GString.h>
#include <fileNames.h>

// Get home directory path.
GString *getHomeDir() {
#ifdef VMS
  return new GString("SYS$LOGIN:");
#else
  char *s;
  struct passwd *pw;
  GString *ret;

  if ((s = getenv("HOME"))) {
    ret = new GString(s);
  } else {
    if ((s = getenv("USER")))
      pw = getpwnam(s);
    else
      pw = getpwuid(getuid());
    if (pw)
      ret = new GString(pw->pw_dir);
    else
      ret = new GString(".");
  }
  return ret;
#endif
}

GString *appendToPath(GString *path, char *fileName) {
#ifdef VMS
  path->append(fileName);
  return path;
#else
  if (path->getLength() > 0 &&
      path->getChar(path->getLength() - 1) != '/')
    path->append('/');
  path->append(fileName);
  return path;
#endif
}

GString *grabPath(char *fileName) {
#ifdef VMS
  char *p;

  if ((p = strrchr(fileName, ']')))
    return new GString(fileName, p + 1 - fileName);
  if ((p = strrchr(fileName, ':')))
    return new GString(fileName, p + 1 - fileName);
  return new GString();
#else
  char *p;

  if ((p = strrchr(fileName, '/')))
    return new GString(fileName, p - fileName);
  return new GString();
#endif
}

GBool isAbsolutePath(char *path) {
#ifdef VMS
  return strchr(path, ':') ||
	 (path[0] == '[' && path[1] != '.');
#else
  return path[0] == '/';
#endif
}
