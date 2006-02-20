#include <cc3_ilp.h>
#include <cc3.h>
#include <stdbool.h>



uint8_t cc3_load_img_rows( cc3_image_t *img, uint16_t rows )
{


}

void cc3_get_pixel(cc3_image_t *img, uint16_t x, uint16_t y, cc3_pixel_t *out_pix)
{
if(img->channels>1)
{
	out_pix.channel[0]=(uint8_t)img->pix[y*img->width+(x*3)]; 
	out_pix.channel[1]=(uint8_t)img->pix[y*img->width+(x*3)+1]; 
	out_pix.channel[2]=(uint8_t)img->pix[y*img->width+(x*3)+2]; 
} 
else {
	out_pix.channel[0]=(uint8_t)img->pix[y*img->width+x]; 
}

}


void cc3_set_pixel(cc3_image_t *img, uint16_t x, uint16_t y, cc3_pixel_t *in_pix)
{
if(img->channels>1)
{
	(uint8_t)img->pix[y*img->width+(x*3)]=in_pix.channel[0]; 
	(uint8_t)img->pix[y*img->width+(x*3)+1]=in_pix.channel[1]; 
	(uint8_t)img->pix[y*img->width+(x*3)+2]=in_pix.channel[2]; 
}
else {
	(uint8_t)img->pix[y*img->width+x]=in_pix.channel[0]; 
}


}

/**
 * cc3_track_color()
 *
 * This function takes parameters set in the  cc3_track_pkt_t structure
 * and fills in the missing parameters with tracked color data.
 * This operates on the image directly from the FIFO.
 *
 * Returns: 0 if the bounds fail, 1 upon sucess
 *
 * Check if num_pixels is greater than 1 to see if an object was detected.
 *
 */
uint8_t cc3_track_color(cc3_track_pkt_t *pkt)
{
uint32_t mm_x,mm_y;
if( pkt->lower_bound.channel[0]>pkt->upper_bound.channel[0] |
    pkt->lower_bound.channel[1]>pkt->upper_bound.channel[1] |
    pkt->lower_bound.channel[2]>pkt->upper_bound.channel[2] ) return 0;
pkt->num_pixels=0;
pkt->x0=UINT16_MAX;
pkt->y0=UINT16_MAX;
pkt->x1=0;
pkt->y1=0;
mm_x=0;
mm_y=0;
for(y=0; y<cc3_g_current_frame.height; y++ )
for(x=0; x<cc3_g_current_frame.width; x++ )
{
	bool pixel_good=0;
	cc3_pixbuf_read();
	if(cc3_g_current_frame.coi==CC3_ALL ) {	
		if(pkt->cc3_g_current_pixel.channel[0]>=pkt->lower_bound.channel[0] && 
	   	pkt->cc3_g_current_pixel.channel[0]<=pkt->upper_bound.channel[0] && 
		pkt->cc3_g_current_pixel.channel[1]>=pkt->lower_bound.channel[1] && 
	   	pkt->cc3_g_current_pixel.channel[1]<=pkt->upper_bound.channel[1] && 
		pkt->cc3_g_current_pixel.channel[2]>=pkt->lower_bound.channel[2] && 
	   	pkt->cc3_g_current_pixel.channel[2]<=pkt->upper_bound.channel[2] ) pixel_good=1; 
	} else
	{
		if(cc3_g_current_pixel.channel[cc3_g_current_frame.coi]>=pkt->lower_bound.channel[cc3_g_current_frame.coi] && 
	   	cc3_g_current_pixel.channel[cc3_g_current_frame.coi]<=pkt->upper_bound.channel[cc3_g_current_frame.coi] ) pixel_good=1; 
	}

	if(pixel_good)
	{
	pkt->num_pixels++;
	if(pkt->x0 > x ) pkt->x0=x;	
	if(pkt->y0 > y ) pkt->y0=y;	
	if(pkt->x1 < x ) pkt->x1=x;	
	if(pkt->y1 < y ) pkt->y1=y;	
	mm_x+=x;
	mm_y+=y;
	}
}

pkt->centroid_x= mm_x / cc3_g_current_frame.width;
pkt->centroid_y= mm_y / cc3_g_current_frame.height;

// FIXME:  Density hack to keep it an integer
if(pkt->num_pixels>0 )
	pkt->int_density=(pixels*1000) / ((pkt->x1 - pkt->x0)*(pkt->y1 - pkt->y0));
else pkt->int_density=0;
return 1;
}

uint8_t cc3_track_color_img(cc3_image_t *img, cc3_track_pkt_t *pkt)
{




}

