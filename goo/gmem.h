/*
 * gmem.h
 *
 * Memory routines with out-of-memory checking.
 *
 * Copyright 1996 Derek B. Noonburg
 */

#ifndef GMEM_H
#define GMEM_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Same as malloc, but prints error message and exits if malloc()
 * returns NULL.
 */
extern void *gmalloc(int size);

/*
 * Same as realloc, but prints error message and exits if realloc()
 * returns NULL.  If <p> is NULL, calls malloc instead of realloc().
 */
extern void *grealloc(void *p, int size);

/*
 * Same as free, but checks for and ignores NULL pointers.
 */
extern void gfree(void *p);

/*
 * Allocate memory and copy a string into it.
 */
extern char *copyString(char *s);

#ifdef __cplusplus
}
#endif

#endif
