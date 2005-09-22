#include "LPC2100.h"
#include "cc3.h"
#include "interrupt.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include "serial.h"
#include "jpeg.c"

void image_send_direct (int size_x, int size_y);


int main ()
{
    unsigned int i = 0;
    int val;

    cc3_system_setup ();

    cc3_io_init (115200);
    cc3_camera_init ();
    cc3_set_colorspace(CC3_YCRCB);

    printf ("Camera Starting up\r\n");
    cc3_set_led (true);
    


    for (i = 0; i < 50; i++) {
        cc3_pixbuf_load ();
    }

    initialize_and_run_jpeg();
   /* 
    printf ("Sending Image\r\n");
    while (1) {
        image_send_direct (cc3_g_current_frame.width,
                           cc3_g_current_frame.height);
    }

*/
    return 0;
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
