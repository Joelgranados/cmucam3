#ifndef CC3_COLOR_TRACK_H
#define CC3_COLOR_TRACK_H

#include <stdint.h>
#include "cc3.h"
#include "cc3_ilp.h"

// MAX_BINARY_WIDTH * 32 (bits/int)
#define MAX_BINARY_WIDTH  20 


typedef struct {
    uint16_t x0,y0,x1,y1;
    uint16_t centroid_x, centroid_y;
    uint32_t num_pixels;
    uint32_t int_density;
    uint8_t noise_filter;
    uint32_t binary_scanline[MAX_BINARY_WIDTH];
    cc3_pixel_t upper_bound;
    cc3_pixel_t lower_bound;
} cc3_track_pkt_t;

uint8_t cc3_track_color_scanline_start(cc3_track_pkt_t *pkt);
uint8_t cc3_track_color_scanline(cc3_image_t *img, cc3_track_pkt_t *pkt);
uint8_t cc3_track_color_scanline_finish(cc3_track_pkt_t *pkt);

uint8_t cc3_track_color(cc3_track_pkt_t *pkt);
//uint8_t cc3_track_color_img(cc3_image_t *img, cc3_track_pkt_t *pkt);

#endif
