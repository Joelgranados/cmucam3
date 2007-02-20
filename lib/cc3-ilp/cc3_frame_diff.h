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


#ifndef CC3_FRAME_DIFF_H
#define CC3_FRAME_DIFF_H

#include <stdint.h>
#include "cc3.h"
#include "cc3_ilp.h"


typedef struct {
    uint16_t x0,y0,x1,y1;
    uint32_t centroid_x,centroid_y;
    uint32_t num_pixels;
    uint32_t int_density;
    uint32_t *previous_template;
    uint32_t *current_template;
    uint16_t template_width, template_height;
    uint8_t threshold;
    uint16_t total_x, total_y;
    uint8_t coi;
    bool load_frame;
    uint32_t _scratch_y;
    uint16_t _bin_div_x;
    uint16_t _bin_div_y;
} cc3_frame_diff_pkt_t;

uint8_t cc3_frame_diff_scanline_start(cc3_frame_diff_pkt_t *pkt);
uint8_t cc3_frame_diff_scanline(cc3_image_t *img, cc3_frame_diff_pkt_t *pkt);
uint8_t cc3_frame_diff_scanline_finish(cc3_frame_diff_pkt_t *pkt);

#endif
