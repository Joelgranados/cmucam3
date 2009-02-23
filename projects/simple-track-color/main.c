#include <stdio.h>
#include <stdlib.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include <cc3_color_track.h>


void simple_track_color(cc3_track_pkt_t* t_pkt);

int main(void) {
  cc3_track_pkt_t t_pkt;


  cc3_uart_init (0, 
		 CC3_UART_RATE_115200,
		 CC3_UART_MODE_8N1,
		 CC3_UART_BINMODE_TEXT);

  cc3_camera_init ();


  //cc3_camera_set_colorspace(CC3_COLORSPACE_YCRCB);
  cc3_camera_set_resolution(CC3_CAMERA_RESOLUTION_LOW);
  //cc3_pixbuf_frame_set_subsample(CC3_SUBSAMPLE_NEAREST, 2, 2);
  

  // init pixbuf with width and height
  cc3_pixbuf_load();

  // Load in your tracking parameters
  t_pkt.lower_bound.channel[CC3_CHANNEL_RED] = 150;
  t_pkt.upper_bound.channel[CC3_CHANNEL_RED] = 255;
  t_pkt.lower_bound.channel[CC3_CHANNEL_GREEN] = 0;
  t_pkt.upper_bound.channel[CC3_CHANNEL_GREEN] = 50;
  t_pkt.lower_bound.channel[CC3_CHANNEL_BLUE] = 0;
  t_pkt.upper_bound.channel[CC3_CHANNEL_BLUE] = 50; 
  t_pkt.noise_filter = 2; 
  t_pkt.track_invert = false;
  
  while(true) {
    simple_track_color(&t_pkt);
    printf( "centroid = %lu,%lu bounding box = %d,%d,%d,%d num pix= %lu density = %lu\n",
		    t_pkt.centroid_x, t_pkt.centroid_y,
		    t_pkt.x0,t_pkt.y0,t_pkt.x1,t_pkt.y1,
		    t_pkt.num_pixels, t_pkt.int_density );

   }
  
}


void simple_track_color(cc3_track_pkt_t * t_pkt)
{
  cc3_image_t img;

  img.channels = 3;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = 1;               // image will hold just 1 row for scanline processing
  img.pix = cc3_malloc_rows (1);
  if (img.pix == NULL) {
    return;
  }

    cc3_pixbuf_load ();
    if (cc3_track_color_scanline_start (t_pkt) != 0) {
      while (cc3_pixbuf_read_rows (img.pix, 1)) {
	   // This does the HSV conversion 
	   // cc3_rgb2hsv_row(img.pix,img.width);
           cc3_track_color_scanline (&img, t_pkt);
          }
        }
    cc3_track_color_scanline_finish (t_pkt);

  free (img.pix);
  return;
}



