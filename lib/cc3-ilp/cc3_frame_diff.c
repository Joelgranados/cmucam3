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
#include "cc3_frame_diff.h"
#include <stdbool.h>
#include <stdio.h>



uint8_t cc3_frame_diff_scanline_start(cc3_frame_diff_pkt_t *pkt)
{
uint8_t x,y;

if(pkt->previous_template==NULL ) return 0;
//if((pkt->load_frame==0) && pkt->current_template==NULL ) return 0;

	// clear frame
	for(y=0; y<pkt->template_height; y++ )
	for(x=0; x<pkt->template_width; x++ )
	{
	if(pkt->load_frame)
 		((uint32_t *)pkt->previous_template)[(y*pkt->template_width)+x]=0;
	else
 		((uint32_t *)pkt->current_template)[(y*pkt->template_width)+x]=0;
	}
pkt->_scratch_y=0;
pkt->_bin_div_x=(pkt->total_x/pkt->template_width);
pkt->_bin_div_y=(pkt->total_y/pkt->template_height);

return 1;
}

uint8_t cc3_frame_diff_scanline(cc3_image_t *img, cc3_frame_diff_pkt_t *pkt)
{
uint32_t x,y;
uint16_t t_x,t_y;
cc3_pixel_t cp;


for(y=pkt->_scratch_y; y<(pkt->_scratch_y+img->height); y++ )
{
t_y=y/pkt->_bin_div_y;
for(x=0; x<img->width; x++ )
{
	uint8_t i;
	cc3_get_pixel( img, x, y-pkt->_scratch_y, &cp );
	if(pkt->load_frame)
		{
		   t_x=x/pkt->_bin_div_x;
 		   ((uint32_t *)pkt->previous_template)[(t_y*pkt->template_width)+t_x]+=cp.channel[0];
		}	
}
}
pkt->_scratch_y=y;

return 1;
}



uint8_t cc3_frame_diff_scanline_finish(cc3_frame_diff_pkt_t *pkt)
{
uint16_t x,y;
uint32_t cube_size;

        cube_size=pkt->_bin_div_x * pkt->_bin_div_y;
	//cube_size=(pkt->total_x/pkt->template_width)*(pkt->total_y/pkt->template_height);
	// divide each square in template by the total number of pixels
	for(y=0; y<pkt->template_height; y++ )
	{
	for(x=0; x<pkt->template_width; x++ )
	{
		if(pkt->load_frame) {
 		   ((uint32_t *)pkt->previous_template)[(y*pkt->template_width)+x]=
 		   ((uint32_t *)pkt->previous_template)[(y*pkt->template_width)+x]/cube_size;
		} else {
 		   ((uint32_t *)pkt->current_template)[(y*pkt->template_width)+x]=
 		   ((uint32_t *)pkt->current_template)[(y*pkt->template_width)+x]/cube_size;
	            // generate the bounding box etc here	
		
		}
	}
	}
return 1;
}



