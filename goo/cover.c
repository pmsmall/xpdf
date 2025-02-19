/*
 * cover.c
 *
 * Simple code coverage checking.
 *
 * Copyright 1996 Derek B. Noonburg
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <gmem.h>
#include <cover.h>

#ifdef COVER

typedef struct _CoverEntry {
  char *name;
  int num;
  struct _CoverEntry *next;
} CoverEntry;

static CoverEntry **hashTab;
static int hashSize;

void doCoverInit(int hashSize1) {
  int h;

  hashSize = hashSize1;
  hashTab = gmalloc(hashSize * sizeof(CoverEntry *));
  for (h = 0; h < hashSize; ++h)
    hashTab[h] = NULL;
}

void doCover(char *name) {
  int h;
  char *p;
  CoverEntry *e;

  h = 0;
  for (p = name; *p; ++p)
    h = (h * 17 + *p) % hashSize;
  for (e = hashTab[h]; e; e = e->next) {
    if (!strcmp(name, e->name))
      break;
  }
  if (!e) {
    e = gmalloc(sizeof(CoverEntry));
    e->name = name;
    e->num = 0;
    e->next = hashTab[h];
    hashTab[h] = e;
  }
  ++e->num;
}

void doCoverDump(FILE *f) {
  int h;
  CoverEntry *e1, *e2;

  fprintf(f, "Coverage:\n");
  for (h = 0; h < hashSize; ++h) {
    for (e1 = hashTab[h]; e1; e1 = e1->next)
      fprintf(stderr, "%-32s %8d\n", e1->name, e1->num);
  }
  fprintf(f, "\n");

  for (h = 0; h < hashSize; ++h) {
    for (e1 = hashTab[h]; e1; e1 = e2) {
      e2 = e1->next;
      gfree(e1);
    }
  }
  gfree(hashTab);
}

#endif
