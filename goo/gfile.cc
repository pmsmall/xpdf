//========================================================================
//
// gfile.cc
//
// Miscellaneous file and directory name manipulation.
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#ifndef VMS
#include <pwd.h>
#endif
#if defined(VMS) && (__DECCXX_VER < 50200000)
#include <unixlib.h>
#endif
#include <GString.h>
#include <gfile.h>

// Some systems don't define this, so just make it something reasonably
// large.
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

//------------------------------------------------------------------------

GString *getHomeDir() {
#ifdef VMS
  //---------- VMS ----------
  return new GString("SYS$LOGIN:");

#elif defined(__EMX__)
  //---------- OS/2+EMX ----------
  char *s;
  GString *ret;

  if ((s = getenv("HOME")))
    ret = new GString(s);
  else
    ret = new GString(".");
  return ret;

#else
  //---------- Unix ----------
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

GString *getCurrentDir() {
  char buf[PATH_MAX+1];

#ifdef __EMX__
  if (!_getcwd2(buf, sizeof(buf)))
#else
  if (!getcwd(buf, sizeof(buf)))
#endif
    return new GString();
  return new GString(buf);
}

GString *appendToPath(GString *path, char *fileName) {
#ifdef VMS
  //---------- VMS ----------
  //~ this should handle everything necessary for file
  //~ requesters, but it's certainly not complete
  char *p0, *p1, *p2;
  char *q1;

  p0 = path->getCString();
  p1 = p0 + path->getLength() - 1;
  if (!strcmp(fileName, "-")) {
    if (*p1 == ']') {
      for (p2 = p1; p2 > p0 && *p2 != '.' && *p2 != '['; --p2) ;
      if (*p2 == '[')
	++p2;
      path->del(p2 - p0, p1 - p2);
    } else if (*p1 == ':') {
      path->append("[-]");
    } else {
      path->clear();
      path->append("[-]");
    }
  } else if ((q1 = strrchr(fileName, '.')) && !strncmp(q1, ".DIR;", 5)) {
    if (*p1 == ']') {
      path->insert(p1 - p0, '.');
      path->insert(p1 - p0 + 1, fileName, q1 - fileName);
    } else if (*p1 == ':') {
      path->append('[');
      path->append(']');
      path->append(fileName, q1 - fileName);
    } else {
      path->clear();
      path->append(fileName, q1 - fileName);
    }
  } else {
    if (*p1 != ']' && *p1 != ':')
      path->clear();
    path->append(fileName);
  }
  return path;

#else
  //---------- Unix and OS/2+EMX ----------
  int i;

  // appending "." does nothing
  if (!strcmp(fileName, "."))
    return path;

  // appending ".." goes up one directory
  if (!strcmp(fileName, "..")) {
    for (i = path->getLength() - 2; i >= 0; --i) {
#ifdef __EMX__
      if (path->getChar(i) == '/' || path->getChar(i) == '\\' ||
	  path->getChar(i) == ':')
#else
      if (path->getChar(i) == '/')
#endif
	break;
    }
    if (i >= 0) {
#ifdef __EMX__
      if (path->getChar(i) == ':')
	++i;
#endif
      path->del(i, path->getLength() - i);
    } else {
      path->clear();
      path->append("..");
    }
    return path;
  }

  // otherwise, append "/" and new path component
#ifdef __EMX__
  if (path->getLength() > 0 &&
      path->getChar(path->getLength() - 1) != '/' &&
      path->getChar(path->getLength() - 1) != '\\')
#else
  if (path->getLength() > 0 &&
      path->getChar(path->getLength() - 1) != '/')
#endif
    path->append('/');
  path->append(fileName);
  return path;
#endif
}

GString *grabPath(char *fileName) {
#ifdef VMS
  //---------- VMS ----------
  char *p;

  if ((p = strrchr(fileName, ']')))
    return new GString(fileName, p + 1 - fileName);
  if ((p = strrchr(fileName, ':')))
    return new GString(fileName, p + 1 - fileName);
  return new GString();

#elif defined(__EMX__)
  //---------- OS/2+EMX ----------
  char *p;

  if ((p = strrchr(fileName, '/')))
    return new GString(fileName, p - fileName);
  if ((p = strrchr(fileName, '\\')))
    return new GString(fileName, p - fileName);
  if ((p = strrchr(fileName, ':')))
    return new GString(fileName, p + 1 - fileName);
  return new GString();

#else
  //---------- Unix ----------
  char *p;

  if ((p = strrchr(fileName, '/')))
    return new GString(fileName, p - fileName);
  return new GString();
#endif
}

GBool isAbsolutePath(char *path) {
#ifdef VMS
  //---------- VMS ----------
  return strchr(path, ':') ||
	 (path[0] == '[' && path[1] != '.' && path[1] != '-');

#elif defined(__EMX__)
  //---------- OS/2+EMX ----------
  return path[0] == '/' || path[0] == '\\' || path[1] == ':';

#else
  //---------- Unix ----------
  return path[0] == '/';
#endif
}

GString *makePathAbsolute(GString *path) {
#ifdef VMS
  //---------- VMS ----------
  char buf[PATH_MAX+1];

  if (!isAbsolutePath(path->getCString())) {
    if (getcwd(buf, sizeof(buf))) {
      if (path->getChar(0) == '[') {
	if (path->getChar(1) != '.')
	  path->insert(0, '.');
	path->insert(0, buf);
      } else {
	path->insert(0, '[');
	path->insert(1, ']');
	path->insert(1, buf);
      }
    }
  }
  return path;

#else
  //---------- Unix and OS/2+EMX ----------
  struct passwd *pw;
  char buf[PATH_MAX+1];
  GString *s;
  char *p1, *p2;
  int n;

  if (path->getChar(0) == '~') {
    if (path->getChar(1) == '/' ||
#ifdef __EMX__
	path->getChar(1) == '\\' ||
#endif
	path->getLength() == 1) {
      path->del(0, 1);
      s = getHomeDir();
      path->insert(0, s);
      delete s;
    } else {
      p1 = path->getCString() + 1;
#ifdef __EMX__
      for (p2 = p1; *p2 && *p2 != '/' && *p2 != '\\'; ++p2) ;
#else
      for (p2 = p1; *p2 && *p2 != '/'; ++p2) ;
#endif
      if ((n = p2 - p1) > PATH_MAX)
	n = PATH_MAX;
      strncpy(buf, p1, n);
      buf[n] = '\0';
      if ((pw = getpwnam(buf))) {
	path->del(0, p2 - p1 + 1);
	path->insert(0, pw->pw_dir);
      }
    }
  } else if (!isAbsolutePath(path->getCString())) {
    if (getcwd(buf, sizeof(buf))) {
      path->insert(0, '/');
      path->insert(0, buf);
    }
  }
  return path;
#endif
}

//------------------------------------------------------------------------
// GDir and GDirEntry
//------------------------------------------------------------------------

GDirEntry::GDirEntry(char *dirPath, char *name1, GBool doStat) {
#ifdef VMS
  char *p;
#else
  struct stat st;
  GString *s;
#endif

  name = new GString(name1);
  dir = gFalse;
  if (doStat) {
#ifdef VMS
    if (!strcmp(name1, "-") ||
	((p = strrchr(name1, '.')) && !strncmp(p, ".DIR;", 5)))
      dir = gTrue;
#else
    s = new GString(dirPath);
    appendToPath(s, name1);
    if (stat(s->getCString(), &st) == 0)
      dir = S_ISDIR(st.st_mode);
    delete s;
#endif
  }
}

GDirEntry::~GDirEntry() {
  delete name;
}

GDir::GDir(char *name, GBool doStat1) {
  path = new GString(name);
  doStat = doStat1;
  dir = opendir(name);
#ifdef VMS
  needParent = strchr(name, '[') != NULL;
#endif
}

GDir::~GDir() {
  delete path;
  if (dir)
    closedir(dir);
}

GDirEntry *GDir::getNextEntry() {
  struct dirent *ent;
  GDirEntry *e;

  e = NULL;
  if (dir) {
#ifdef VMS
    if (needParent) {
      e = new GDirEntry(path->getCString(), "-", doStat);
      needParent = gFalse;
      return e;
    }
#endif
    ent = readdir(dir);
#ifndef VMS
    if (ent && !strcmp(ent->d_name, "."))
      ent = readdir(dir);
#endif
    if (ent)
      e = new GDirEntry(path->getCString(), ent->d_name, doStat);
  }
  return e;
}

void GDir::rewind() {
  if (dir)
    rewinddir(dir);
#ifdef VMS
  needParent = strchr(path->getCString(), '[') != NULL;
#endif
}
