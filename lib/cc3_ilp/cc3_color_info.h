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
