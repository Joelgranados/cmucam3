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


#ifndef CC3_HSV_H
#define CC3_HSV_H

#include <stdint.h>
#include "cc3.h"
#include "cc3_ilp.h"


inline void cc3_rgb2hsv (cc3_pixel_t * pix);
void cc3_rgb2hsv_row (cc3_pixel_t * pix, uint16_t size);

#endif
