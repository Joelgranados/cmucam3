#ifndef _CONV_H_
#define _CONV_H_

#include "cc3_ilp.h"
#include <stdint.h>

#define MAX_KERNEL_SIZE 3

typedef struct{
uint8_t mat[MAX_KERNEL_SIZE][MAX_KERNEL_SIZE];
uint8_t divisor;
uint32_t size;
} cc3_kernel_t;


int cc3_convolve_img(cc3_image_t *img, cc3_kernel_t kernel);

#endif
