#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "png.h"

#include "cc3.h"

static void capture_png(FILE *f);

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
  cc3_led_set_state(1, true);
  i = 0;
  while(!cc3_button_get_state());
  while(true) {
    char filename[16];

    // Check if files exist, if they do then skip over them
    do {
#ifdef VIRTUAL_CAM
      snprintf(filename, 16, "img%.5d.png", i);
#else
      snprintf(filename, 16, "c:/img%.5d.png", i);
#endif

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
	cc3_led_set_state(2, true);
	cc3_timer_wait_ms(500);
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
    capture_png(f);

    result = fclose(f);
    if (result) {
      perror("second fclose failed");
    }
    fprintf(stderr, "\r\n");

    i++;
  }

  return 0;
}


void capture_png(FILE *f)
{
  uint32_t x, y;
  uint32_t size_x, size_y;

  uint32_t time, time2;
  int write_time;

  cc3_pixbuf_load();

  // init PNG
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
						NULL,
						NULL,
						NULL);
  if (!png_ptr) {
    fprintf(stderr, "png_struct\n");
    exit(1);
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr,
			     (png_infopp)NULL);
    fprintf(stderr, "png_info\n");
    exit(1);
  }

  uint8_t *row = cc3_malloc_rows(1);

  size_x = cc3_g_pixbuf_frame.width;
  size_y = cc3_g_pixbuf_frame.height;

  // these are the max values that work within
  // the memory constraints of LPC2106
  //
  // mem_usage = (1 << (window_bits + 2)) + (1 << (mem_level + 9))
  png_set_compression_mem_level(png_ptr, 5);
  png_set_compression_window_bits(png_ptr, 11);

  // more png
  png_init_io(png_ptr, f);
  png_set_IHDR(png_ptr, info_ptr, size_x, size_y,
	       8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_DEFAULT,
	       PNG_FILTER_TYPE_DEFAULT);


  // header
  png_write_info(png_ptr, info_ptr);


  time = cc3_timer_get_current_ms();
  for (y = 0; y < size_y; y++) {
    cc3_pixbuf_read_rows(row, 1);

    png_write_row(png_ptr, row);

    fprintf(stderr, ".");
    fflush(stderr);
  }
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  time2 = cc3_timer_get_current_ms();
  write_time = time2 - time;

  free(row);

  fprintf(stderr, "\n"
          "write_time  %10d\n",
          write_time);
}
