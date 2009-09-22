/*
 * Copyright 2006-2007  Anthony Rowe and Adam Goode
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


#include "cc3.h"
#include "cc3_ilp.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t cc3_load_img_rows (cc3_image_t * img, uint16_t rows)
{


}

inline void cc3_get_pixel (cc3_image_t * img, uint16_t x, uint16_t y,
                    cc3_pixel_t * out_pix)
{
  if (img->channels > 1) {
    out_pix->channel[0] = ((uint8_t *) img->pix)[(y * img->width * 3) + (x * 3)];
    out_pix->channel[1] =
      ((uint8_t *) img->pix)[(y * img->width*3) + (x * 3) + 1];
    out_pix->channel[2] =
      ((uint8_t *) img->pix)[(y * img->width*3) + (x * 3) + 2];
  }
  else {
    out_pix->channel[0] = ((uint8_t *) img->pix)[y * img->width + x];
  }

}


inline void cc3_set_pixel (cc3_image_t * img, uint16_t x, uint16_t y,
                    cc3_pixel_t * in_pix)
{
  if (img->channels > 1) {
    ((uint8_t *) img->pix)[(y * img->width * 3) + (x * 3)] = in_pix->channel[0];
    ((uint8_t *) img->pix)[(y * img->width * 3) + (x * 3) + 1] = in_pix->channel[1];
    ((uint8_t *) img->pix)[(y * img->width * 3) + (x * 3) + 2] = in_pix->channel[2];
  }
  else {
    ((uint8_t *) img->pix)[y * img->width + x] = in_pix->channel[0];
  }


}
