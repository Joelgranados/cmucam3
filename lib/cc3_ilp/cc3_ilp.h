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
