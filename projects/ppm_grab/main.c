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

  // setup system
  cc3_system_setup();
  cc3_uart_init (0,
		 CC3_UART_RATE_115200,
		 CC3_UART_MODE_8N1,
		 CC3_UART_BINMODE_TEXT);

  cc3_camera_init ();

 // cc3_set_colorspace(CC3_YCRCB);
  cc3_set_resolution(CC3_HIGH_RES);
  // cc3_pixbuf_set_subsample (CC3_NEAREST, 2, 2);
  cc3_wait_ms(1000);

  // init
  cc3_set_led(1);
  i = 0;
  while(!cc3_read_button());
  while(true) {
    char filename[16];

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
	perror("crap");
      } else {
	fprintf(stderr, "full\n");
      }

      while (true) {
	cc3_set_led(2);
	cc3_wait_ms(500);
	cc3_clr_led(2);
	cc3_wait_ms(500);
      }
    }


    if (light_on) {
      cc3_set_led (2);
    } else {
      cc3_clr_led (2);
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
  uint8_t *row = cc3_malloc_rows(1);

  size_x = cc3_g_current_frame.width;
  size_y = cc3_g_current_frame.height;

  fprintf(f,"P6\n%d %d\n255\n",size_x,size_y );
  cc3_pixbuf_load ();

  for (y = 0; y < size_y; y++) {
    cc3_pixbuf_read_rows(row, 1);

    for (x = 0; x < size_x * 3U; x++) {
      uint8_t p = row[x];
      fputc(p, f);
    }
    fprintf(stderr, ".");
    fflush(stderr);
  }
  free(row);
}
