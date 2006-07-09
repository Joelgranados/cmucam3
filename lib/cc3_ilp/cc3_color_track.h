/*
 * Copyright 2006  Anthony Rowe
 *
 * This file is part of cc3.
 *
 * cc3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * cc3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cc3; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
