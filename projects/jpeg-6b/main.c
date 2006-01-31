#include <stdio.h>
#include <stdlib.h>

#include "cc3.h"
#include "jpeglib.h"


static void capture_current_jpeg(FILE *f);
static void init_jpeg(void);
static void destroy_jpeg(void);

int main(void) {
  int i;
  FILE *f;

  // setup system    
  cc3_system_setup ();
  cc3_uart0_init (115200,UART_8N1,UART_STDOUT);

  cc3_camera_init ();
   
  cc3_set_colorspace(CC3_YCRCB);
  cc3_set_resolution(CC3_HIGH_RES);

  cc3_wait_ms(1000);

  // init jpeg
  init_jpeg();

  i = 0;
  while(true) {
    while(!cc3_read_button());
    
    char filename[16];
    snprintf(filename, 16, "c:/img%.5d.jpg", i);
    printf("%s\r\n", filename);
    f = fopen(filename, "w");

    capture_current_jpeg(f);

    fclose(f);
    
    i++;
  }
  

  destroy_jpeg();
  return 0;
}


static struct jpeg_compress_struct cinfo;
static struct jpeg_error_mgr jerr;
static cc3_pixel_t *row;

void init_jpeg(void) {
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  // parameters for jpeg image
  cinfo.image_width = cc3_g_current_frame.width;
  cinfo.image_height = cc3_g_current_frame.height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_YCbCr;

  // set image quality, etc.
  jpeg_set_defaults(&cinfo);

  // allocate memory for 1 row
  row = malloc(sizeof(cc3_pixel_t) * cc3_g_current_frame.width);
}

void capture_current_jpeg(FILE *f) {
  JSAMPROW row_pointer[1];
  row_pointer[0] = row;

  // output is file
  jpeg_stdio_dest(&cinfo, f);

  // capture a frame to the FIFO
  cc3_pixbuf_load();

  // read and compress
  jpeg_start_compress(&cinfo, TRUE);
  while (cinfo.next_scanline < cinfo.image_height) {
    cc3_pixbuf_read_rows(row, cinfo.image_width, 1);
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  
  // finish
  jpeg_finish_compress(&cinfo);
}



void destroy_jpeg(void) {
  jpeg_destroy_compress(&cinfo);
  free(row);
}
