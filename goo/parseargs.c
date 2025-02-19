/*
 * parseargs.h
 *
 * Command line argument parser.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <parseargs.h>

static ArgDesc *findArg(ArgDesc *args, char *arg);
static Boolean grabArg(ArgDesc *arg, int i, int *argc, char *argv[]);

Boolean parseArgs(ArgDesc *args, int *argc, char *argv[]) {
  ArgDesc *arg;
  int i, j;
  Boolean ok;

  ok = true;
  i = 1;
  while (i < *argc) {
    if (!strcmp(argv[i], "--")) {
      --*argc;
      for (j = i; j < *argc; ++j)
	argv[j] = argv[j+1];
      break;
    } else if ((arg = findArg(args, argv[i]))) {
      if (!grabArg(arg, i, argc, argv))
	ok = false;
    } else {
      ++i;
    }
  }
  return ok;
}

void printUsage(char *program, char *otherArgs, ArgDesc *args) {
  ArgDesc *arg;

  fprintf(stderr, "Usage: %s", program);
  for (arg = args; arg->arg; ++arg) {
    switch (arg->kind) {
    case argFlag:
      fprintf(stderr, " [%s]", arg->arg);
      break;
    case argInt:
      fprintf(stderr, " [%s <int>]", arg->arg);
      break;
    case argFP:
      fprintf(stderr, " [%s <fp>]", arg->arg);
      break;
    case argString:
      fprintf(stderr, " [%s <string>]", arg->arg);
      break;
    }
  }
  if (otherArgs)
    fprintf(stderr, " %s", otherArgs);
  fprintf(stderr, "\n");
  for (arg = args; arg->arg; ++arg) {
    if (arg->usage)
      fprintf(stderr, "  %-8s: %s\n", arg->arg, arg->usage);
  }
}

static ArgDesc *findArg(ArgDesc *args, char *arg) {
  ArgDesc *p;

  for (p = args; p->arg; ++p) {
    if (!strcmp(p->arg, arg))
      return p;
  }
  return NULL;
}

static Boolean grabArg(ArgDesc *arg, int i, int *argc, char *argv[]) {
  int n;
  int j;
  Boolean ok;

  ok = true;
  n = 0;
  switch (arg->kind) {
  case argFlag:
    *(Boolean *)arg->val = true;
    n = 1;
    break;
  case argInt:
    if (i + 1 < *argc && isInt(argv[i+1])) {
      *(int *)arg->val = atoi(argv[i+1]);
      n = 2;
    } else {
      ok = false;
      n = 1;
    }
    break;
  case argFP:
    if (i + 1 < *argc && isFP(argv[i+1])) {
      *(double *)arg->val = atof(argv[i+1]);
      n = 2;
    } else {
      ok = false;
      n = 1;
    }
    break;
  case argString:
    if (i + 1 < *argc) {
      strncpy((char *)arg->val, argv[i+1], arg->size - 1);
      ((char *)arg->val)[arg->size - 1] = '\0';
      n = 2;
    } else {
      ok = false;
      n = 1;
    }
  }
  if (n > 0) {
    *argc -= n;
    for (j = i; j < *argc; ++j)
      argv[j] = argv[j+n];
  }
  return ok;
}

Boolean isInt(char *s) {
  if (*s == '-' || *s == '+')
    ++s;
  while (isdigit(*s))
    ++s;
  if (*s)
    return false;
  return true;
}

Boolean isFP(char *s) {
  int n;

  if (*s == '-' || *s == '+')
    ++s;
  n = 0;
  while (isdigit(*s)) {
    ++s;
    ++n;
  }
  if (*s == '.')
    ++s;
  while (isdigit(*s)) {
    ++s;
    ++n;
  }
  if (n > 0 && (*s == 'e' || *s == 'E')) {
    ++s;
    if (*s == '-' || *s == '+')
      ++s;
    n = 0;
    if (!isdigit(*s))
      return false;
    do {
      ++s;
    } while (isdigit(*s));
  }
  if (*s)
    return false;
  return true;
}
