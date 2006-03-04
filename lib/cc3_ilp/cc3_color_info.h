#ifndef CC3_COLOR_INFO_H
#define CC3_COLOR_INFO_H

#include <stdint.h>
#include "cc3.h"
#include "cc3_ilp.h"



typedef struct {
    cc3_pixel_t mean;
    cc3_pixel_t deviation;
    cc3_pixel_t min,max;
    uint16_t scratch_x,scratch_y;
    uint32_t scratch_mean[3];
    uint32_t scratch_pix;
} cc3_color_info_pkt_t;

uint8_t cc3_color_info_scanline_start(cc3_color_info_pkt_t *pkt);
uint8_t cc3_color_info_scanline(cc3_image_t *img, cc3_color_info_pkt_t *pkt);
uint8_t cc3_color_info_scanline_finish(cc3_color_info_pkt_t *pkt);

#endif
