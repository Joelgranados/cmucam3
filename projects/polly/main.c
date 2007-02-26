#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include <cc3_img_writer.h>
#include "polly.h"


int main (void)
{
  uint32_t last_time, val,i;
  char c;
  uint8_t *x_axis,*h,cnt,conf;
  polly_config_t p_config;

  cc3_filesystem_init();

  // configure uarts
  cc3_uart_init (0, CC3_UART_RATE_115200, CC3_UART_MODE_8N1,
                 CC3_UART_BINMODE_BINARY);
  // Make it so that stdout and stdin are not buffered
  val = setvbuf (stdout, NULL, _IONBF, 0);
  val = setvbuf (stdin, NULL, _IONBF, 0);

  cc3_camera_init ();

  cc3_camera_set_colorspace (CC3_COLORSPACE_RGB);
  cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_LOW);
  cc3_camera_set_auto_white_balance (true);
  cc3_camera_set_auto_exposure (true);


  cc3_pixbuf_frame_set_subsample (CC3_SUBSAMPLE_NEAREST, 2, 2);
  cc3_pixbuf_frame_set_coi (CC3_CHANNEL_GREEN);

  cc3_led_set_state (0, false);
  cc3_led_set_state (1, false);
  cc3_led_set_state (2, false);

  // sample wait command in ms 
  cc3_timer_wait_ms (1000);

  // initialize pixbuf
  cc3_pixbuf_load();

  x_axis = malloc(cc3_g_pixbuf_frame.width);
  h = malloc(cc3_g_pixbuf_frame.width);
         
  p_config.color_thresh=10;
  p_config.min_blob_size=20;
  p_config.connectivity=1;
  p_config.horizontal_edges=1;
  p_config.vertical_edges=1;
  p_config.blur=1;
  p_config.histogram=malloc(cc3_g_pixbuf_frame.width);


  while (1) {
	cc3_pixbuf_load();
        polly(p_config);  
	// p_config.histogram gets filled with the return data

	printf( "Histogram:\r" );
    	for(i=0; i<cc3_g_pixbuf_frame.width; i++ )
	{
		printf( "%d %d\r",i,p_config.histogram[i] );
	}
	printf( "\r\r" );

	}

  return 0;
}

