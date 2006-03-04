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
 pkt->deviation.channel[i]=0;
}
pkt->scratch_x=0;
pkt->scratch_y=0;

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
	if(cc3_g_current_frame.coi==CC3_ALL ) {	
	
	
	} else
	{
	
	
	}

}

pkt->scratch_y=y;

return 1;
}



uint8_t cc3_color_info_scanline_finish(cc3_color_info_pkt_t *pkt)
{

return 1;
}



