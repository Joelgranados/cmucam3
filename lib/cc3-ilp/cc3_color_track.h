/*
 * Copyright 2006  Anthony Rowe and Adam Goode
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef CC3_COLOR_TRACK_H
#define CC3_COLOR_TRACK_H

#include <stdint.h>
#include "cc3.h"
#include "cc3_ilp.h"

// MAX_BINARY_WIDTH * 8 (bits/int)
#define MAX_BINARY_WIDTH  80 


typedef struct {
    uint16_t x0,y0,x1,y1;
    uint32_t centroid_x, centroid_y;
    uint32_t num_pixels;
    uint32_t int_density;
    uint8_t noise_filter;
    bool track_invert;
    uint8_t binary_scanline[MAX_BINARY_WIDTH];
    cc3_pixel_t upper_bound;
    cc3_pixel_t lower_bound;
    uint16_t scratch_x,scratch_y;
} cc3_track_pkt_t;

uint8_t cc3_track_color_scanline_start(cc3_track_pkt_t *pkt);
uint8_t cc3_track_color_scanline(cc3_image_t *img, cc3_track_pkt_t *pkt);
uint8_t cc3_track_color_scanline_finish(cc3_track_pkt_t *pkt);

uint8_t cc3_track_color(cc3_track_pkt_t *pkt);
//uint8_t cc3_track_color_img(cc3_image_t *img, cc3_track_pkt_t *pkt);

#endif
