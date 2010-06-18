#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "png.h"

#include "cc3.h"
#include "cc3_debug.h"

/* FIXME: This should be defined at compile time.  There is not general way
 *        of defining vars through the make files.  Comment the line out if
 *        you want pngcam to act like png-grab
 */
#define PNGCAM_MODE_CAM

#ifndef VIRTUAL_CAM

static void capture_png( FILE* );
static bool get_next_file_name( char* );

static int file_offset = 0;

int main(void) {
  FILE *f;
  char filename[16];

  cc3_debug_init();

  cc3_camera_init ();

  cc3_filesystem_init(); //for the MMC.

  cc3_camera_set_resolution(CC3_CAMERA_RESOLUTION_HIGH);
  cc3_timer_wait_ms(1000);

  // init
  cc3_led_set_state(1, true);

  while(true) {
#ifdef PNGCAM_MODE_CAM
    // Wait until the user pushes the button.
    while(!cc3_button_get_state());
#endif

    get_next_file_name(filename);

    f = fopen(filename, "w");
    if (f == NULL){
      CC3_ERROR("Could not open %s.", filename);
      continue; // We will try and try again.
    }

    // We actually take the pucture.
    CC3_PDEBUG("%s", "Capturing png");
    capture_png(f);

    // Close the png file
    if (fclose(f))
      CC3_ERROR("Could not close %s", filename);
  }

  return 0;
}

//FIXME: There could be an issue here with the allowed number of
//       file sin the filesustem.  Check the upper renge.
//       There was code in png-grab that suggested that 512 was
//       the maximum amount of files.
bool get_next_file_name( char* file )
{
  FILE *f;

  //Find the next free filename.
  do {
    snprintf(file, 16, "c:/img%.5d.png", file_offset);

    f = fopen(file, "r");
    if (f != NULL) {
      file_offset++;
      if (fclose(f))
        CC3_ERROR("Could not close %s", file);
    }
  } while(f != NULL);

  // We have found a filename
  CC3_PDEBUG("Next file name: %s", file); // print the filename to the serial.
  return true;
}

void capture_png(FILE *f)
{
  uint32_t y;
  uint32_t size_x, size_y;

  uint32_t time1, time2;
  int write_time;

  cc3_pixbuf_load();

  // init PNG
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                NULL,
                                                NULL,
                                                NULL);
  if (!png_ptr) {
    CC3_ERROR("%s", "Error in png_struct");
    exit(1);
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr,
                             (png_infopp)NULL);
    CC3_ERROR("%s", "Error in png_info");
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


  time1 = cc3_timer_get_current_ms();
  for (y = 0; y < size_y; y++) {
    cc3_pixbuf_read_rows(row, 1);

    png_write_row(png_ptr, row);

    // FIXME: This is a hack to output periods without a line break.
    //        We should probably create a special debug level with
    //        no line break.
    //fprintf(stderr, ".");
    //fflush(stderr);
  }
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  time2 = cc3_timer_get_current_ms();
  write_time = time2 - time1;

  free(row);

  CC3_PDEBUG("\n write_time %10d", write_time);
}

#endif
