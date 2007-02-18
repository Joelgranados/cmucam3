#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

#include <cc3.h>

static const int print_increment = 100;

int main (void)
{
  void *buf;
  uint32_t last_time;
  int i;
  int next_print;
  
  // configure uarts
  cc3_uart_init (0, 
		 CC3_UART_RATE_115200,
		 CC3_UART_MODE_8N1,
		 CC3_UART_BINMODE_TEXT);
  
  cc3_camera_init ();
  
  cc3_camera_set_colorspace(CC3_COLORSPACE_RGB);
  cc3_camera_set_resolution(CC3_CAMERA_RESOLUTION_LOW);
  cc3_camera_set_auto_white_balance(true);
  cc3_camera_set_auto_exposure(true);
  
  buf = cc3_malloc_rows(1);
  if (!buf) {
    printf("ERROR!\n");
    exit(1);
  }

  printf("Running benchmark...\n");

  i = 0;
  next_print = print_increment;
  last_time = cc3_timer_get_current_ms();

  while(true) {
    cc3_pixbuf_load();

    while(cc3_pixbuf_read_rows(buf, 1));
    
    i++;

    if (i >= next_print) {
      double fps = print_increment / ((cc3_timer_get_current_ms() - last_time) / 1000.0);
      printf("%d frames, %g fps\n", i, fps);
      
      next_print += print_increment;
      last_time = cc3_timer_get_current_ms();
    }
  }

  free(buf);
  return 0;
}


