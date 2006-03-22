#include "cc3.h"
#include "cc3_ilp.h"
#include "cc3_color_track.h"
#include <stdbool.h>
#include <stdio.h>

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
  uint16_t y, x;

  x=pkt->scratch_x;
  y=pkt->scratch_y;

  if( (pkt->lower_bound.channel[0]>pkt->upper_bound.channel[0]) ||
      (pkt->lower_bound.channel[1]>pkt->upper_bound.channel[1]) ||
      (pkt->lower_bound.channel[2]>pkt->upper_bound.channel[2]) ) return 0;
  pkt->num_pixels=0;
  pkt->x0=UINT16_MAX;
  pkt->y0=UINT16_MAX;
  pkt->x1=0;
  pkt->y1=0;
  pkt->centroid_x=0;
  pkt->centroid_y=0;


  for(y=0; y<cc3_g_current_frame.height; y++ )
    for(x=0; x<cc3_g_current_frame.width; x++ )
      {
	bool pixel_good=0;
	cc3_pixbuf_read();
	if(cc3_g_current_frame.coi==CC3_ALL ) {	
	  if(cc3_g_current_pixel.channel[0]>=pkt->lower_bound.channel[0] && 
	     cc3_g_current_pixel.channel[0]<=pkt->upper_bound.channel[0] && 
	     cc3_g_current_pixel.channel[1]>=pkt->lower_bound.channel[1] && 
	     cc3_g_current_pixel.channel[1]<=pkt->upper_bound.channel[1] && 
	     cc3_g_current_pixel.channel[2]>=pkt->lower_bound.channel[2] && 
	     cc3_g_current_pixel.channel[2]<=pkt->upper_bound.channel[2] ) pixel_good=1; 
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
	    pkt->centroid_x+=x;
	    pkt->centroid_y+=y;
	  }
      }


  if(pkt->num_pixels>0 )
    {
      // FIXME:  Density hack to keep it an integer
      pkt->int_density=(pkt->num_pixels*1000) / ((pkt->x1 - pkt->x0)*(pkt->y1 - pkt->y0));
      pkt->centroid_x= pkt->centroid_x / pkt->num_pixels;
      pkt->centroid_y= pkt->centroid_y / pkt->num_pixels;

    }
  else 
    {
      pkt->int_density=0;
      pkt->x0=0;
      pkt->y0=0;
      pkt->x1=0;
      pkt->y1=0;
      pkt->centroid_x=0;
      pkt->centroid_y=0;

    }
  return 1;
}

uint8_t cc3_track_color_img(cc3_image_t *img, cc3_track_pkt_t *pkt)
{


}


uint8_t cc3_track_color_scanline_start(cc3_track_pkt_t *pkt)
{

  if( (pkt->lower_bound.channel[0]>pkt->upper_bound.channel[0]) ||
      (pkt->lower_bound.channel[1]>pkt->upper_bound.channel[1]) ||
      (pkt->lower_bound.channel[2]>pkt->upper_bound.channel[2]) ) return 0;
  pkt->num_pixels=0;
  pkt->x0=UINT16_MAX;
  pkt->y0=UINT16_MAX;
  pkt->x1=0;
  pkt->y1=0;
  pkt->centroid_x=0;
  pkt->centroid_y=0;
  pkt->scratch_x=0;
  pkt->scratch_y=0;
  return 1;
}

uint8_t cc3_track_color_scanline(cc3_image_t *img, cc3_track_pkt_t *pkt)
{
  uint32_t x,y;

  for(x=0; x<MAX_BINARY_WIDTH; x++ ) pkt->binary_scanline[x]=0;

  for(y=pkt->scratch_y; y<(pkt->scratch_y+img->height); y++ )
    for(x=0; x<img->width; x++ )
      {
	bool pixel_good=0;
	cc3_pixel_t cp;
	//cc3_pixbuf_read();
	cc3_get_pixel( img, x, 0, &cp );	
	if(cc3_g_current_frame.coi==CC3_ALL ) {	
	  if(cp.channel[0]>=pkt->lower_bound.channel[0] && 
	     cp.channel[0]<=pkt->upper_bound.channel[0] && 
	     cp.channel[1]>=pkt->lower_bound.channel[1] && 
	     cp.channel[1]<=pkt->upper_bound.channel[1] && 
	     cp.channel[2]>=pkt->lower_bound.channel[2] && 
	     cp.channel[2]<=pkt->upper_bound.channel[2] ) pixel_good=1; 
	} else
	  {
	    if(cp.channel[cc3_g_current_frame.coi]>=pkt->lower_bound.channel[cc3_g_current_frame.coi] && 
	       cp.channel[cc3_g_current_frame.coi]<=pkt->upper_bound.channel[cc3_g_current_frame.coi] ) pixel_good=1; 
	  }

	/*	pkt->binary_scanline[0]=0x01020304; 
	  pkt->binary_scanline[1]=0x05060708; 
	  pkt->binary_scanline[2]=0x090A0B0C; 
	  pkt->binary_scanline[3]=0x0D0E0F10; 
	  pkt->binary_scanline[4]=0x11121314; 
	*/	if(pixel_good)
	  {
	    uint8_t block,offset;
	    block=x/8;
	    offset=x%8;
	    offset=7-offset;
	    pkt->binary_scanline[block] |= (1<<offset);
	    pkt->num_pixels++;
	    if(pkt->x0 > x ) pkt->x0=x;	
	    if(pkt->y0 > y ) pkt->y0=y;	
	    if(pkt->x1 < x ) pkt->x1=x;	
	    if(pkt->y1 < y ) pkt->y1=y;	
	    pkt->centroid_x+=x;
	    pkt->centroid_y+=y;
	  }
      }

  pkt->scratch_y=y;

  return 1;
}



uint8_t cc3_track_color_scanline_finish(cc3_track_pkt_t *pkt)
{
  if(pkt->num_pixels>0 )
    {
      // FIXME:  Density hack to keep it an integer
      pkt->int_density=(pkt->num_pixels*1000) / ((pkt->x1 - pkt->x0)*(pkt->y1 - pkt->y0));
      pkt->centroid_x= pkt->centroid_x / pkt->num_pixels;
      pkt->centroid_y= pkt->centroid_y / pkt->num_pixels;

    }
  else 
    {
      pkt->int_density=0;
      pkt->x0=0;
      pkt->y0=0;
      pkt->x1=0;
      pkt->y1=0;
      pkt->centroid_x=0;
      pkt->centroid_y=0;

    }


}



