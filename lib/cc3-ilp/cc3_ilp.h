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


#ifndef CC3_ILP_H
#define CC3_ILP_H

#include <stdint.h>
#include "cc3.h"

typedef struct {
    uint16_t width, height;
    uint8_t channels;
    uint8_t depth;  // This can be used for binary images etc in the future
    void* pix;
} cc3_image_t;


void cc3_get_pixel(cc3_image_t *img, uint16_t x, uint16_t y, cc3_pixel_t *out_pix);
void cc3_set_pixel(cc3_image_t *img, uint16_t x, uint16_t y, cc3_pixel_t *in_pix);


#endif
