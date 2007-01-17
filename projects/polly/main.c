#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include "polly.h"

#define MMC_DEBUG




/* simple hello world, showing features and compiling*/
int main (void)
{
  uint32_t last_time, val;
  char c;


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

  while (1) {
	polly_config_t p_config;
        p_config.color_thresh=20;
        p_config.min_blob_size=20;
        p_config.connectivity=0;
        p_config.horizontal_edges=0;
        p_config.vertical_edges=1;
        p_config.blur=1;
        polly(p_config);
	
	}

  return 0;
}

