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

  cc3_uart_init (0, 
		 CC3_UART_RATE_115200,
		 CC3_UART_MODE_8N1,
		 CC3_UART_BINMODE_TEXT);

  cc3_camera_init ();

  cc3_filesystem_init();

  //cc3_set_colorspace(CC3_COLORSPACE_YCRCB);
  cc3_camera_set_resolution(CC3_CAMERA_RESOLUTION_HIGH);
  // cc3_pixbuf_set_subsample (CC3_SUBSAMPLE_NEAREST, 2, 2);
  cc3_timer_wait_ms(1000);

  // init pixbuf with width and height
  cc3_pixbuf_load();

  // init jpeg
  init_jpeg();

  cc3_led_set_state(1, true);
  i = 0;
  while(true) {
    char filename[16];
    cc3_led_set_state(1, false);
    while(!cc3_button_get_state());
    cc3_led_set_state(1, true);
  
   // Check if files exist, if they do then skip over them 
    do { 
#ifdef VIRTUAL_CAM
        snprintf(filename, 16, "img%.5d.jpg", i);
#else
	snprintf(filename, 16, "c:/img%.5d.jpg", i);
#endif
        f = fopen(filename, "r");
    	if(f!=NULL ) { 
		printf( "%s already exists...\n",filename ); 
		i++; 
		fclose(f);
		}
    } while(f!=NULL);

    // print file that you are going to write to stderr
    fprintf(stderr,"%s\r\n", filename);
    f = fopen(filename, "w");
    if(f==NULL || i>200 )
    {
      cc3_led_set_state(3, true);
	printf( "Error: Can't open file\r\n" );
	while(1);
    }
    capture_current_jpeg(f);

    fclose(f);
    
    i++;
  }
  

  destroy_jpeg();
  return 0;
}


static struct jpeg_compress_struct cinfo;
static struct jpeg_error_mgr jerr;
//static cc3_pixel_t *row;
uint8_t *row;

void init_jpeg(void) {
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  // parameters for jpeg image
  cinfo.image_width = cc3_g_pixbuf_frame.width;
  cinfo.image_height = cc3_g_pixbuf_frame.height;
  printf( "image width=%d image height=%d\n", cinfo.image_width, cinfo.image_height );
  cinfo.input_components = 3;
 // cinfo.in_color_space = JCS_YCbCr;
  cinfo.in_color_space = JCS_RGB;

  // set image quality, etc.
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 80, true);

  // allocate memory for 1 row
  row = cc3_malloc_rows(1);
  if(row==NULL) printf( "Out of memory!\n" );
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
    cc3_pixbuf_read_rows(row, 1);
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  
  // finish
  jpeg_finish_compress(&cinfo);
}



void destroy_jpeg(void) {
  jpeg_destroy_compress(&cinfo);
  free(row);
}
