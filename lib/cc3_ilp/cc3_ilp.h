#ifndef CC3_ILP_H
#define CC3_ILP_H

#include <stdint.h>
#include "cc3.h"

typedef struct {
    uint16_t width, height;
    uint8_t channels;
    void* pix;
} cc3_image_t;


void cc3_send_image_direct(void);
void cc3_get_pixel(cc3_image_t *img, uint16_t x, uint16_t y, cc3_pixel_t *out_pix);
void cc3_set_pixel(cc3_image_t *img, uint16_t x, uint16_t y, cc3_pixel_t *in_pix);


#endif
