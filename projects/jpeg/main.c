#include "LPC2100.h"
#include "cc3.h"
#include "interrupt.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include "serial.h"
#include "jpeg.c"

void image_send_direct (int size_x, int size_y);
void ppm_send_direct (int size_x, int size_y);


int main ()
{
    unsigned int i = 0;
    int val;

    cc3_system_setup ();

    cc3_uart0_init (115200,UART_8N1, UART_STDOUT);
    cc3_camera_init ();

    printf ("Camera Starting up\r\n");
    cc3_set_led (true);
    //cc3_set_colorspace(CC3_YCRCB);
    cc3_set_resolution(CC3_HIGH_RES);
    printf( "Image width = %d, Image height = %d\n",cc3_g_current_frame.width, cc3_g_current_frame.height );

    cc3_wait_ms(1000);
  //  ppm_send_direct (cc3_g_current_frame.width,cc3_g_current_frame.height);
   // while(1);

    initialize_and_run_jpeg();
   /* 
    printf ("Sending Image\r\n");
    while (1) {
        image_send_direct (cc3_g_current_frame.width,
                           cc3_g_current_frame.height);
    }

*/
    while(1);
    return 0;
}

void ppm_send_direct (int size_x, int size_y)
{
    int x, y, val;
    cc3_pixbuf_load ();
    printf ("P3\n%d %d\n255\n", size_x, size_y);
    for (y = 0; y < size_y; y++) {
        for (x = 0; x < size_x; x++) {
            val = cc3_pixbuf_read ();
            //if(val==0) { printf( "Damn, error in pixbuf_read()...\r\n" ); while(1); }
            printf ("%d %d %d  ", cc3_g_current_pixel.channel[CC3_RED],
                    cc3_g_current_pixel.channel[CC3_GREEN],
                    cc3_g_current_pixel.channel[CC3_BLUE]);
        }
        printf ("\n");
    }

}

void image_send_direct (int size_x, int size_y)
{
    int x, y;
    cc3_pixbuf_load ();
    putchar (1);
    putchar (size_x);
    if (size_y > 255)
        size_y = 255;
    putchar (size_y);
    for (y = 0; y < size_y; y++) {
        putchar (2);
        for (x = 0; x < size_x; x++) {
            cc3_pixbuf_read ();
            putchar (cc3_g_current_pixel.channel[CC3_RED]);
            putchar (cc3_g_current_pixel.channel[CC3_GREEN]);
            putchar (cc3_g_current_pixel.channel[CC3_BLUE]);
        }
    }
    putchar (3);
    fflush (stdout);
}
