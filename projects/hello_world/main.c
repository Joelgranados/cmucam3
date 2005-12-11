#include "LPC2100.h"
#include "cc3.h"
#include "interrupt.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include "serial.h"

void image_send_direct (int size_x, int size_y);
void image_touch (int size_x, int size_y);
void ppm_send_direct (int size_x, int size_y);


int main ()
{
    unsigned int i = 0;
    int val;
    cc3_system_setup ();


    //system_setup ();
    cc3_io_init (115200);
    cc3_camera_init ();
    printf ("CMUcam3 Starting up\r\n");
    cc3_set_led (true);
/*
val=0;
while(val!=2106)
{
    printf( "Type a number, or 2106 to break...\r\n");
    scanf( "%d",&val );
    printf( "You typed %d\r\n",val);
}*/
 

   int blah;

while(1)
{
   // printf( "timer= %d %d\r\n",clock(),clock()/CLOCKS_PER_SEC); 
    printf( "timer= %d\r\n",clock()); 
}

    /* for (i = 0; i < 50; i++) {
       printf ("Try load\r\n");
       cc3_pixbuf_load ();
       printf ("loaded..\r\n");
       } */
    printf ("Sending Image\r\n");
    //if( cc3_pixbuf_set_roi( 30,30,50,60 )==0 ) printf( "Error Setting region of interest\r\n" );
    if (cc3_pixbuf_set_subsample (CC3_NEAREST, 1, 3) == 0)
        printf ("Error Setting Subsample Mode\r\n");
    //if( cc3_pixbuf_set_roi( 2,20,40,50 )==0 ) printf( "Error Setting region of interest\r\n" );

    while (1) {
        //image_send_direct (cc3_g_current_frame.width,
        scanf ("%d", &val);
        ppm_send_direct (cc3_g_current_frame.width,
                         cc3_g_current_frame.height);
    }


    return 0;
}

void image_touch (int size_x, int size_y)
{
    int x, y, val;

    cc3_pixbuf_load ();
    for (y = 0; y < size_y; y++) {
        for (x = 0; x < size_x; x++) {
            //val=cc3_pixbuf_read ();
            _cc3_pixbuf_read_all ();
            //putchar (cc3_g_current_pixel.channel[CC3_RED]);
            //putchar (cc3_g_current_pixel.channel[CC3_GREEN]);
            //putchar (cc3_g_current_pixel.channel[CC3_BLUE]);
        }
    }
    //putchar (3);
    //fflush (stdout);
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
    fflush (stdout);

}


void image_send_direct (int size_x, int size_y)
{
    int x, y, val;
    cc3_pixbuf_load ();
    putchar (1);
    putchar (size_x);
    if (size_y > 255)
        size_y = 255;
    putchar (size_y);
    for (y = 0; y < size_y; y++) {
        putchar (2);
        for (x = 0; x < size_x; x++) {
            val = cc3_pixbuf_read ();
            if (val == 0) {
                printf ("Damn, error in pixbuf_read()...\r\n");
                while (1);
            }
            putchar (cc3_g_current_pixel.channel[CC3_RED]);
            putchar (cc3_g_current_pixel.channel[CC3_GREEN]);
            putchar (cc3_g_current_pixel.channel[CC3_BLUE]);
        }
    }
    putchar (3);
    fflush (stdout);
}
