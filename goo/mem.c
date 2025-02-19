/*
 * mem.c
 *
 * Memory routines with out-of-memory checking.
 *
 * Copyright 1996 Derek B. Noonburg
 */

#include <stdio.h>
#include <stdlib.h>
#include <mem.h>

void *smalloc(int size) {
  void *p;

  if (size == 0)
    return NULL;
  if (!(p = malloc(size))) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
  }
  return p;
}

void *srealloc(void *p, int size) {
  void *q;

  if (size == 0) {
    if (p)
      free(p);
    return NULL;
  }
  if (p)
    q = realloc(p, size);
  else
    q = malloc(size);
  if (!q) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
  }
  return q;
}

void sfree(void *p) {
  if (p)
    free(p);
}

char *copyString(char *s) {
  char *s1;

  s1 = smalloc(strlen(s) + 1);
  strcpy(s1, s);
  return s1;
}
