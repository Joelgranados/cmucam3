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
	if(cc3_g_current_frame.coi==CC3_ALL ) {	
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
	        i=cc3_g_current_frame.coi;
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



