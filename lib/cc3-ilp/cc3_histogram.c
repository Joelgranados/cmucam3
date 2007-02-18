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


#include "cc3.h"
#include "cc3_ilp.h"
#include "cc3_histogram.h"
#include <stdbool.h>
#include <stdio.h>



uint8_t cc3_histogram_scanline_start(cc3_histogram_pkt_t *pkt)
{
uint8_t i;

if(pkt->hist==NULL ) return 0;
if(cc3_g_pixbuf_frame.coi!=CC3_CHANNEL_ALL)
	pkt->channel=0; //cc3_g_pixbuf_frame.coi;
for(i=0; i<pkt->bins; i++ )
{
 pkt->hist[i]=0;
}
pkt->scratch_y=0;
pkt->bin_div=(MAX_PIXEL_VALUE/pkt->bins);
return 1;
}

uint8_t cc3_histogram_scanline(cc3_image_t *img, cc3_histogram_pkt_t *pkt)
{
uint32_t x,y;
cc3_pixel_t cp;


for(y=pkt->scratch_y; y<(pkt->scratch_y+img->height); y++ )
for(x=0; x<img->width; x++ )
{
	uint8_t i;
	cc3_get_pixel( img, x, y-pkt->scratch_y, &cp );	
	i=cp.channel[pkt->channel]/pkt->bin_div;
	if(i>0) i--;
	if(i>=pkt->bins) i=pkt->bins-1;
	pkt->hist[i]++;
}
pkt->scratch_y=y;

return 1;
}



uint8_t cc3_histogram_scanline_finish(cc3_histogram_pkt_t *pkt)
{
// We don't need to do anything...
return 1;
}



