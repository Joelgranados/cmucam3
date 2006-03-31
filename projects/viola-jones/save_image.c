#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>

int main(void)
{
FILE *fp;
cc3_pixel_t my_pix;
cc3_image_t img;

   // setup system      
   cc3_system_setup ();

   // configure uarts
   cc3_uart_init (0, CC3_UART_RATE_115200,CC3_UART_MODE_8N1,CC3_UART_BINMODE_BINARY);
   
   // Make it so that stdout and stdin are not buffered
   val=setvbuf(stdout, NULL, _IONBF, 0 );
   val=setvbuf(stdin, NULL, _IONBF, 0 );
   cc3_camera_init ();
 
   cc3_set_colorspace(CC3_RGB);
   cc3_set_resolution(CC3_LOW_RES);
   cc3_set_auto_white_balance(true);
   cc3_set_auto_exposure(true);

   img.channels=1;
   img.width=cc3_g_current_frame.width;
   img.height=1;  // image will hold just 1 row for scanline processing
   img.pix = malloc(3 * img.width);
   fp=fopen("c:/test.pgm","w" );
   fprintf( fp, "P2\n%d %d\n255\n", cc3_g_current_frame.width, cc3_g_current_frame.height );
   pixbuf_load();

   for( uint16_t y=0; y<cc3_g_current_frame.height; y++ )
     {
       cc3_pixbuf_read_rows(img.pix, img.width, 1);
       for( uint16_t x=0; x<cc3_g_current_frame.width; x++ )
	 {
	   cc3_get_pixel( &img, x, 0, &my_pix );
	   fprintf( fp,"%d ",my_pix.channel[0] );
	 }
       fprintf( fp, "\n" );
     }

   fclose(fp);

}
