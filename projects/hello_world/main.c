#include "LPC2100.h"
#include "cc3.h"
#include "interrupt.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "serial.h"
#include "servo.h"

void image_send_direct (int size_x, int size_y);
void image_touch (int size_x, int size_y);
void ppm_send_direct (int size_x, int size_y);
void ppm_send_mem(int size_x, int size_y, cc3_pixel_t *img);

cc3_pixel_t my_img[80][140];

int main ()
{
    uint32_t i = 0;
    uint32_t cnt = 1;
    int32_t val;
    

    
    cc3_system_setup ();
    cc3_io_init (115200);
    cc3_camera_init ();
    printf ("CMUcam3 Starting up\n");
    cc3_set_led (true);


    cc3_servo_init ();
    printf ("timer= %d\n", clock ());
    
    printf ("Setting up Image Parameters\n");
    if( cc3_pixbuf_set_roi( 0,0,80,140 )==0 ) printf( "Error Setting region of interest\n" );
    //if (cc3_pixbuf_set_subsample (CC3_NEAREST, 1, 1) == 0) printf ("Error Setting Subsample Mode\n");
    if (cc3_pixbuf_set_coi(CC3_ALL) == 0) printf ("Error Setting Channel of Interest\n");

    while (1) {
        scanf ("%d", &val);
	cc3_pixbuf_load();

	if( cc3_pixbuf_read_rows( &my_img, 140 )== 0) 
		printf( "Fuck, pixbuf read returned some madness!\n" );
	ppm_send_mem(cc3_g_current_frame.width, 140, &my_img ); 
	
	//ppm_send_direct (cc3_g_current_frame.width, cc3_g_current_frame.height);
    }


    return 0;
}

void image_touch (int size_x, int size_y)
{
    uint32_t x, y, val;

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

void ppm_send_mem(int size_x, int size_y, cc3_pixel_t *img)
{
    uint32_t x, y, val;
    printf ("P3\n%d %d\n255\n", size_x, size_y);
    for (y = 0; y < size_y; y++) {
        for (x = 0; x < size_x; x++) {
	    uint32_t i;
	    i=(y*size_x)+x;
            printf ("%d %d %d  ", img[i].channel[CC3_RED],
                    img[i].channel[CC3_GREEN],
                    img[i].channel[CC3_BLUE]);
        }
        printf ("\n");
    }
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
}
