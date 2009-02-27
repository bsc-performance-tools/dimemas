#ifndef __aleatorias_h
#define __aleatorias_h

/**
 * External routines defined in file aleatorias.c
 **/
extern void srandomine(unsigned int seed1, unsigned int seed2);
extern float rnorm(float p1, float p2, float p3, float p4);
extern float erlng(float p1, float p2, float p3, float p4);
extern float expo(float mean, float min, float max);
extern float unform(float min, float max);

#endif
