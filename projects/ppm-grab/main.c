#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "cc3.h"


void capture_ppm(FILE *fp);

int main(void) {
  int i;
  int result;
  FILE *f;
  bool light_on = true;

  cc3_uart_init (0,
		 CC3_UART_RATE_115200,
		 CC3_UART_MODE_8N1,
		 CC3_UART_BINMODE_TEXT);

  cc3_camera_init ();

  // use MMC
  cc3_filesystem_init();

  cc3_camera_set_resolution(CC3_CAMERA_RESOLUTION_HIGH);
  cc3_timer_wait_ms(1000);

  // init
  cc3_led_set_state(0, false);
  i = 0;
  while(true) {
    char filename[16];
  while(!cc3_button_get_state());

    // Check if files exist, if they do then skip over them
    do {
      snprintf(filename, 16, "c:/img%.5d.ppm", i);
      //snprintf(filename, 16, "img%.5d.ppm", i);
      f = fopen(filename, "r");
      if (f != NULL) {
	printf( "%s already exists...\n",filename );
	i++;
	result = fclose(f);
	if (result) {
	  perror("first fclose failed");
	}
      }
    } while(f != NULL);

    // print file that you are going to write to stderr
    fprintf(stderr,"%s ", filename);
    fflush(stderr);
    f = fopen(filename, "w");

    if (f == NULL || i > 512) {
      if (f == NULL) {
	fprintf(stderr, "Error unable to open file. Check MMC card and try rebooting.\n");
      } else {
	fprintf(stderr, "MMC card full\n");
      }

      while (true) {
	cc3_led_set_state(0, true);
	cc3_led_set_state(2, true);
	cc3_timer_wait_ms(500);
	cc3_led_set_state(0, false);
	cc3_led_set_state(2, false);
	cc3_timer_wait_ms(500);
      }
    }


    if (light_on) {
      cc3_led_set_state (2, true);
    } else {
      cc3_led_set_state (2, false);
    }
    light_on = !light_on;
    capture_ppm(f);

    result = fclose(f);
    if (result) {
      perror("second fclose failed");
    }
    fprintf(stderr, "\r\n");

    i++;
  }

  return 0;
}


void capture_ppm(FILE *f)
{
  uint32_t x, y;
  uint32_t size_x, size_y;

  uint32_t time, time2;
  int write_time;

  cc3_pixbuf_load ();
  uint8_t *row = cc3_malloc_rows(1);

  size_x = cc3_g_pixbuf_frame.width;
  size_y = cc3_g_pixbuf_frame.height;

  fprintf(f,"P6\n%d %d\n255\n",size_x,size_y );

  time = cc3_timer_get_current_ms();
  for (y = 0; y < size_y; y++) {
    cc3_pixbuf_read_rows(row, 1);
    for (x = 0; x < size_x * 3U; x++) {
      uint8_t p = row[x];
      if (fputc(p, f) == EOF) {
	perror("fputc failed");
      }
    }
    fprintf(stderr, ".");
    fflush(stderr);
  }
  time2 = cc3_timer_get_current_ms();
  write_time = time2 - time;

  free(row);

  fprintf(stderr, "\n"
	  "write_time  %10d\n",
	  write_time);
}
