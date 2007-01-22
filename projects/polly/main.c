#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include <cc3_math.h>
#include "polly.h"

//#define MMC_DEBUG


/* simple hello world, showing features and compiling*/
int main (void)
{
  uint32_t last_time, val,i;
  char c;
  uint8_t *x_axis;
  polly_config_t p_config;

  // setup system    
  cc3_system_setup ();

  cc3_filesystem_init();

  // configure uarts
  cc3_uart_init (0, CC3_UART_RATE_115200, CC3_UART_MODE_8N1,
                 CC3_UART_BINMODE_BINARY);
  // Make it so that stdout and stdin are not buffered
  val = setvbuf (stdout, NULL, _IONBF, 0);
  val = setvbuf (stdin, NULL, _IONBF, 0);

  cc3_camera_init ();

  cc3_set_colorspace (CC3_RGB);
  cc3_set_resolution (CC3_LOW_RES);
  cc3_set_auto_white_balance (true);
  cc3_set_auto_exposure (true);


  cc3_pixbuf_set_subsample (CC3_NEAREST, 2, 2);
  cc3_pixbuf_set_coi (CC3_GREEN);

  cc3_clr_led (0);
  cc3_clr_led (1);
  cc3_clr_led (2);

  // sample wait command in ms 
  cc3_wait_ms (1000);
  x_axis = malloc(cc3_g_current_frame.width);
         
  p_config.color_thresh=20;
  p_config.min_blob_size=20;
  p_config.connectivity=0;
  p_config.horizontal_edges=0;
  p_config.vertical_edges=1;
  p_config.blur=1;
  p_config.histogram=malloc(cc3_g_current_frame.width); 

  while (1) {
     	double distance;
 	cc3_linear_reg_data_t reg_line;

        polly(p_config);  // p_config.histogram gets filled with the return data

    	for(i=0; i<cc3_g_current_frame.width; i++ )
    		x_axis[i]=i;
    

     	cc3_linear_reg(x_axis, p_config.histogram, cc3_g_current_frame.width,&reg_line);

     	printf( "b=%f\n",reg_line.b );     
     	printf( "m=%f\n",reg_line.m );     
     	printf( "r^2=%f\n",reg_line.r_sqr );     

     	distance=reg_line.m*(cc3_g_current_frame.width/2)+reg_line.b;
     	printf( "distance = %f\n",distance ); 
    
    //	convert_histogram_to_ppm (&polly_img, config.histogram);
	
	}

  return 0;
}

