#ifndef __mallocame_h
#define __mallocame_h
/**
 * External routines defined in file malloc.c
 **/
extern void malloc_init(void);
extern char *mallocame(int s);
extern void freeame(char  *a, int s);
extern void malloc_end();

#endif
