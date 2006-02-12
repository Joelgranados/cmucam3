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
   
 // cc3_set_colorspace(CC3_YCRCB);
  cc3_set_resolution(CC3_HIGH_RES);
 // cc3_pixbuf_set_pixel_mode( CC3_DROP_2ND_GREEN );
  cc3_wait_ms(1000);

  // init jpeg
  init_jpeg();

  i = 0;
  while(true) {
    cc3_clr_led(1);
    while(!cc3_read_button());
    cc3_set_led(1);
    
    char filename[16];
    snprintf(filename, 16, "c:/img%.5d.jpg", i);
    fprintf(stderr,"%s\r\n", filename);
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
//static cc3_pixel_t *row;
uint8_t *row;

void init_jpeg(void) {
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  // parameters for jpeg image
  cinfo.image_width = cc3_g_current_frame.width*2;
  cinfo.image_height = cc3_g_current_frame.height;
  printf( "image width=%d image height=%d\n",cc3_g_current_frame.width, cc3_g_current_frame.height );
  cinfo.input_components = 3;
 // cinfo.in_color_space = JCS_YCbCr;
  cinfo.in_color_space = JCS_RGB;

  // set image quality, etc.
  jpeg_set_quality(&cinfo, 100, true);
  jpeg_set_defaults(&cinfo);

  // allocate memory for 1 row
  //row = malloc(sizeof(cc3_pixel_t) * cc3_g_current_frame.width);
  row = malloc( 2* 3 * cc3_g_current_frame.width);
  if(row==NULL) printf( "FUCK, out of memory!\n" );
}

void capture_current_jpeg(FILE *f) {
  uint32_t i;
  JSAMPROW row_pointer[1];
  row_pointer[0] = row;

  // output is file
  jpeg_stdio_dest(&cinfo, f);

  // capture a frame to the FIFO
  cc3_pixbuf_load();

  // read and compress
  jpeg_start_compress(&cinfo, TRUE);
  while (cinfo.next_scanline < cinfo.image_height) {
       for(i=0; i<(cinfo.image_width*3); i+=6 )
       {
	cc3_pixbuf_read();
	row[i]=cc3_g_current_pixel.channel[CC3_RED];
	row[i+1]=cc3_g_current_pixel.channel[CC3_GREEN];
	row[i+2]=cc3_g_current_pixel.channel[CC3_BLUE];
	row[i+3]=cc3_g_current_pixel.channel[CC3_RED];
	row[i+4]=cc3_g_current_pixel.channel[CC3_GREEN2];
	row[i+5]=cc3_g_current_pixel.channel[CC3_BLUE];
       } 
	  //cc3_pixbuf_read_rows(row, cinfo.image_width, 1);

    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  
  // finish
  jpeg_finish_compress(&cinfo);
}



void destroy_jpeg(void) {
  jpeg_destroy_compress(&cinfo);
  free(row);
}
