/*
 * cover.h
 *
 * Simple code coverage checking.
 */

#ifndef COVER_H
#define COVER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef COVER

#define coverInit(hashSize1) doCoverInit(hashSize1)
#define cover(name) doCover(name)
#define coverDump(f) doCoverDump(f)

extern void doCoverInit(int hashSize1);
extern void doCover(char *name);
extern void doCoverDump(FILE *f);

#else

#define coverInit(hashSize1)
#define cover(name)
#define coverDump(f)

#endif

#ifdef __cplusplus
}
#endif

#endif
