#ifndef _CONV_H_
#define _CONV_H_

#include <cc3_ilp.h>

#define MAX_FILTER_SIZE 3

typedef struct{
uint32_t mat[MAX_FILTER_SIZE][MAX_FILTER_SIZE];
uint32_t size;
} filter_t;


int convolve(cc3_image_t img, filter_t filter);

#endif
