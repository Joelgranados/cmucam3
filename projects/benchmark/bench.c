#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

#include <cc3.h>


int main (void)
{
  void *buf;
  uint32_t last_time, new_time;
  int i;
  
  // setup system    
  cc3_system_setup ();
  
  // configure uarts
  cc3_uart_init (0, 
		 CC3_UART_RATE_115200,
		 CC3_UART_MODE_8N1,
		 CC3_UART_BINMODE_TEXT);
  
  cc3_camera_init ();
  
  cc3_set_colorspace(CC3_RGB);
  cc3_set_resolution(CC3_LOW_RES);
  cc3_set_auto_white_balance(true);
  cc3_set_auto_exposure(true);
  
  buf = malloc(3 * cc3_g_current_frame.width);
  if (!buf) {
    printf("ERROR!\n");
    exit(1);
  }

  last_time = cc3_timer();
  i = 0;

  while(true) {
    int y = 0;

    cc3_pixbuf_load();

    for (y = 0; y < cc3_g_current_frame.height; y++) {
      cc3_pixbuf_read_rows(buf, 
			   cc3_g_current_frame.width, 
			   1);
    }
    
    i++;

    new_time = cc3_timer();
    if (new_time - last_time > 5000) {
      printf("%d: %e fps\n", new_time, i / 5.0);
      
      last_time = new_time;
      i = 0;
    }
  }

  free(buf);
  return 0;
}


