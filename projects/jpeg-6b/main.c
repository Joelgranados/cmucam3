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
  cc3_uart_init (0, 
		 CC3_UART_RATE_115200,
		 CC3_UART_MODE_8N1,
		 CC3_UART_BINMODE_TEXT);

  cc3_camera_init ();
   
  //cc3_set_colorspace(CC3_YCRCB);
  cc3_set_resolution(CC3_HIGH_RES);
  // cc3_pixbuf_set_subsample (CC3_NEAREST, 2, 2);
  cc3_wait_ms(1000);

  // init jpeg
  init_jpeg();

  cc3_set_led(1);
  i = 0;
  while(true) {
    char filename[16];
    cc3_clr_led(1);
    while(!cc3_read_button());
    cc3_set_led(1);
  
   // Check if files exist, if they do then skip over them 
    do { 
    	snprintf(filename, 16, "c:/img%.5d.jpg", i);
    	// For virtual camera use this path...
        // snprintf(filename, 16, "img%.5d.jpg", i);
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
	cc3_set_led(3);
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
  cinfo.image_width = cc3_g_current_frame.width;
  cinfo.image_height = cc3_g_current_frame.height;
  printf( "image width=%d image height=%d\n", cinfo.image_width, cinfo.image_height );
  cinfo.input_components = 3;
 // cinfo.in_color_space = JCS_YCbCr;
  cinfo.in_color_space = JCS_RGB;

  // set image quality, etc.
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 100, true);

  // allocate memory for 1 row
  row = cc3_malloc_rows(1);
  if(row==NULL) printf( "FUCK, out of memory!\n" );
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
