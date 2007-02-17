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


#ifndef CC3_HISTOGRAM_H
#define CC3_HISTOGRAM_H

#include <stdint.h>
#include "cc3.h"
#include "cc3_ilp.h"

#define MAX_PIXEL_VALUE    240

typedef struct {
    cc3_channel_t channel;
    uint32_t bins;
    uint32_t *hist;
    uint32_t scratch_y;
    uint32_t bin_div;
} cc3_histogram_pkt_t;

uint8_t cc3_histogram_scanline_start(cc3_histogram_pkt_t *pkt);
uint8_t cc3_histogram_scanline(cc3_image_t *img, cc3_histogram_pkt_t *pkt);
uint8_t cc3_histogram_scanline_finish(cc3_histogram_pkt_t *pkt);

#endif
