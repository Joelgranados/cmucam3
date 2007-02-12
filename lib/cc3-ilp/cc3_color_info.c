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
#include "cc3_color_info.h"
#include <stdbool.h>
#include <stdio.h>



uint8_t cc3_color_info_scanline_start(cc3_color_info_pkt_t *pkt)
{
uint8_t i;

for(i=0; i<3; i++ )
{
 pkt->mean.channel[i]=0;
 pkt->scratch_mean[i]=0;
 pkt->deviation.channel[i]=0;
 pkt->min.channel[i]=255;
 pkt->max.channel[i]=0;
}
pkt->scratch_x=0;
pkt->scratch_y=0;
pkt->scratch_pix=0;

return 1;
}

uint8_t cc3_color_info_scanline(cc3_image_t *img, cc3_color_info_pkt_t *pkt)
{
uint32_t x,y;


for(y=pkt->scratch_y; y<(pkt->scratch_y+img->height); y++ )
for(x=0; x<img->width; x++ )
{
	cc3_pixel_t cp;
	cc3_get_pixel( img, x, 0, &cp );	
	pkt->scratch_pix++;
	if(cc3_g_pixbuf_frame.coi==CC3_CHANNEL_ALL ) {	
		uint8_t i;
		for(i=0; i<3; i++ )
		{
		pkt->scratch_mean[i]+=cp.channel[i];
		if(cp.channel[i]<pkt->min.channel[i]) pkt->min.channel[i]=cp.channel[i];	
		if(cp.channel[i]>pkt->max.channel[i]) pkt->max.channel[i]=cp.channel[i];
		}	
	
	} else
	{
	    uint8_t i;
	        i=cc3_g_pixbuf_frame.coi;
		pkt->scratch_mean[i]+=cp.channel[i];
		if(cp.channel[i]<pkt->min.channel[i]) pkt->min.channel[i]=cp.channel[i];	
		if(cp.channel[i]>pkt->max.channel[i]) pkt->max.channel[i]=cp.channel[i];
	}

}

pkt->scratch_y=y;

return 1;
}



uint8_t cc3_color_info_scanline_finish(cc3_color_info_pkt_t *pkt)
{
uint8_t i;
for(i=0; i<3; i++ )
{
	pkt->mean.channel[i]=pkt->scratch_mean[i]/pkt->scratch_pix;
	pkt->deviation.channel[i]=((pkt->max.channel[i]-pkt->mean.channel[i])+
		(pkt->mean.channel[i]-pkt->min.channel[i]))/2;
}

return 1;
}



