/*
 * mem.h
 *
 * Memory routines with out-of-memory checking.
 */

#ifndef MEM_H
#define MEM_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Same as malloc, but prints error message and exits if malloc()
 * returns NULL.
 */
extern void *smalloc(int size);

/*
 * Same as realloc, but prints error message and exits if realloc()
 * returns NULL.  If <p> is NULL, calls malloc instead of realloc().
 */
extern void *srealloc(void *p, int size);

/*
 * Same as free, but checks for and ignores NULL pointers.
 */
extern void sfree(void *p);

/*
 * Allocate memory and copy a string into it.
 */
extern char *copyString(char *s);

#ifdef __cplusplus
}
#endif

#endif
