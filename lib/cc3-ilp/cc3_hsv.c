/*
 * Copyright 2007  Anthony Rowe and Adam Goode
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
#include "cc3_hsv.h"
#include <stdbool.h>
#include <stdio.h>




/*
 * This function will convert an RGB pixel into a HSV pixel replacing the original
 * RGB value.
 *
 */
inline void cc3_rgb2hsv (cc3_pixel_t * pix)
{
  uint8_t hue, sat, val;
  uint8_t rgb_min, rgb_max;
  rgb_max = 0;
  rgb_min = 255;
  if (pix->channel[CC3_CHANNEL_RED] > rgb_max)
    rgb_max = pix->channel[CC3_CHANNEL_RED];
  if (pix->channel[CC3_CHANNEL_GREEN] > rgb_max)
    rgb_max = pix->channel[CC3_CHANNEL_GREEN];
  if (pix->channel[CC3_CHANNEL_BLUE] > rgb_max)
    rgb_max = pix->channel[CC3_CHANNEL_BLUE];
  if (pix->channel[CC3_CHANNEL_RED] < rgb_min)
    rgb_min = pix->channel[CC3_CHANNEL_RED];
  if (pix->channel[CC3_CHANNEL_GREEN] < rgb_min)
    rgb_min = pix->channel[CC3_CHANNEL_GREEN];
  if (pix->channel[CC3_CHANNEL_BLUE] < rgb_min)
    rgb_min = pix->channel[CC3_CHANNEL_BLUE];

// compute V
  val = rgb_max;
  if (val == 0) {
    hue = sat = 0;
    pix->channel[CC3_CHANNEL_HUE] = 0;
    pix->channel[CC3_CHANNEL_SAT] = 0;
    pix->channel[CC3_CHANNEL_VAL] = val;
    return;
  }

// compute S
  sat = 255 * (rgb_max - rgb_min) / val;
  if (sat == 0) {
    pix->channel[CC3_CHANNEL_HUE] = 0;
    pix->channel[CC3_CHANNEL_SAT] = 0;
    pix->channel[CC3_CHANNEL_VAL] = val;
    return;
  }

// compute H
  if (rgb_max == pix->channel[CC3_CHANNEL_RED]) {
    hue =
      0 + 43 * (pix->channel[CC3_CHANNEL_GREEN] -
                pix->channel[CC3_CHANNEL_BLUE]) / (rgb_max - rgb_min);
  }
  else if (rgb_max == pix->channel[CC3_CHANNEL_GREEN]) {
    hue =
      85 + 43 * (pix->channel[CC3_CHANNEL_BLUE] -
                 pix->channel[CC3_CHANNEL_RED]) / (rgb_max - rgb_min);
  }
  else {                        /* rgb_max == blue */

    hue =
      171 + 43 * (pix->channel[CC3_CHANNEL_RED] -
                  pix->channel[CC3_CHANNEL_GREEN]) / (rgb_max - rgb_min);
  }
  pix->channel[CC3_CHANNEL_HUE] = hue;
  pix->channel[CC3_CHANNEL_SAT] = sat;
  pix->channel[CC3_CHANNEL_VAL] = val;
}

void cc3_rgb2hsv_row (cc3_pixel_t * pix, uint16_t size)
{
  uint16_t i;

  for (i = 0; i < size; i++)
    cc3_rgb2hsv (&pix[i]);

}
