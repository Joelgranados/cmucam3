#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

#include "cc3.h"

static void image_send_direct (int size_x, int size_y);
static void image_touch (int size_x, int size_y);
static void ppm_send_direct (int size_x, int size_y);
static void ppm_send_direct_p6 (FILE *f, int size_x, int size_y);
static void ppm_send_direct_p2 (int size_x, int size_y);
static void ppm_send_mem(int size_x, int size_y, cc3_pixel_t *img);

static cc3_pixel_t my_img[144][88];

/* very simple hello world, showing features and compiling with 
   separate files */
int main (void)
{
  uint32_t i = 0;
    uint32_t cnt = 1;
    int32_t val;


    FILE *f, *f2;
    int c;

    // setup system    
    cc3_system_setup ();
    cc3_uart_init (0, CC3_UART_RATE_115200,CC3_UART_MODE_8N1,CC3_UART_BINMODE_BINARY);

 //   cc3_uart1_init (9600,UART_8N1,UART_STDERR);
    cc3_camera_init ();
   
    cc3_set_colorspace(CC3_RGB);
    cc3_set_resolution(CC3_HIGH_RES);
    cc3_set_auto_white_balance(true);
    cc3_set_auto_exposure(true);
   
     
    cc3_wait_ms(1000);
    cc3_set_led (0);
    cc3_set_led (1);
    cc3_set_led (2);
    //ppm_send_direct (cc3_g_current_frame.width, cc3_g_current_frame.height);
    //ppm_send_direct_p2(cc3_g_current_frame.width, cc3_g_current_frame.height);

    /*
    printf("initializing MMC...\r\n");
   
    mmc_driver.init();
     
    
      printf("IsValid: %d\r\n"
	     "SectorsPerFAT: %d\r\n"
	     "SectorsPerCluster: %d\r\n"
	     "SectorZero: %d\r\n"
	     "FirstFatSector: %d\r\n"
	     "SecondFatSector: %d\r\n"
	     "RootDirSector: %d\r\n"
	     "NumberRootDirEntries: %d\r\n"
	     "DataStartSector: %d\r\n"
	     "MaxDataSector: %d\r\n",
	     DriveDesc.IsValid,
	     DriveDesc.SectorsPerFAT,
	     DriveDesc.SectorsPerCluster,
	     DriveDesc.SectorZero,
	     DriveDesc.FirstFatSector,
	     DriveDesc.SecondFatSector,
	     DriveDesc.RootDirSector,
	     DriveDesc.NumberRootDirEntries,
	     DriveDesc.DataStartSector,
	     DriveDesc.MaxDataSector);
    */
    //cc3_wait_ms(3000);

      /*
    printf("opening test.txt\r\n");
    f = fopen("c:/test.txt", "r");
    printf("opening test.txt2\r\n");
    f2 = fopen("c:/test2.txt", "w");
    while((c = fgetc(f)) != EOF) {
      putchar(c);
      putc(toupper(c), f2);
    }

    fclose(f);
    fclose(f2);

    printf("\r\n\r\ndone\r\n");
      */

      printf("push button\r\n");
    i = 0;
    while(true) {
      while(!cc3_read_button());

      char filename[32];
      snprintf(filename, 32, "c:/img%.5d.ppm", i);
      printf("%s\r\n", filename);
      f = fopen(filename, "w");
      ppm_send_direct_p6(f, 
			 cc3_g_current_frame.width, 
			 cc3_g_current_frame.height);
      fclose(f);

      i++;
    }
    

   while(1); 


    printf ("CMUcam3 Starting up\n");
    cc3_set_led (true);




    
 //   cc3_wait_ms(500);
//    fprintf(stderr, "Can you hear the CMUcam?\n" );
     
   // cc3_servo_init ();
   // printf ("timer= %d\n", clock());
   // printf ("cc3 timer= %d\n", cc3_timer());

    printf ("Setting up Image Parameters\n");
   // if( cc3_pixbuf_set_roi( 0,0,88,144 )==0 ) printf( "Error Setting region of interest\n" );
   // if (cc3_pixbuf_set_subsample (CC3_NEAREST, 1, 1) == 0) printf ("Error Setting Subsample Mode\n");
   // if (cc3_pixbuf_set_coi(CC3_ALL) == 0) printf ("Error Setting Channel of Interest\n");
   // if (cc3_pixbuf_set_pixel_mode(CC3_DROP_2ND_GREEN) == 0) printf ("Error Setting Pixel Mode\n");

    cc3_set_resolution(CC3_HIGH_RES);

    printf( "width=%d height=%d\n", cc3_g_current_frame.width, cc3_g_current_frame.height);
    ppm_send_direct (cc3_g_current_frame.width, cc3_g_current_frame.height);
    while (1) {
        scanf ("%d", &val);
	cc3_pixbuf_load();
	// The array bounds should match the dimensions provided as well as the width of the actual image
	// The height of the image does not matter since you may want to operate on a slice 
	if( cc3_pixbuf_read_rows( &my_img, cc3_g_current_frame.width, cc3_g_current_frame.height )!=1)
	{	
		printf( "Damn, pixbuf read returned some badness!\n" );
		while(1);
	}
	ppm_send_mem(cc3_g_current_frame.width, cc3_g_current_frame.height, &my_img ); 
	
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
            //_cc3_pixbuf_read_all ();
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
void ppm_send_direct_p6 (FILE *f, int size_x, int size_y)
{
    int x, y, val;
    cc3_pixbuf_load ();
    fprintf (f, "P6 %d %d 255\n", size_x, size_y);
    for (y = 0; y < size_y; y++) {
        for (x = 0; x < size_x; x++) {
            val = cc3_pixbuf_read ();
            //if(val==0) { printf( "Damn, error in pixbuf_read()...\r\n" ); while(1); }
            fprintf (f, "%c%c%c", cc3_g_current_pixel.channel[CC3_RED],
                    cc3_g_current_pixel.channel[CC3_GREEN],
                    cc3_g_current_pixel.channel[CC3_BLUE]);
        }
     //printf( "\n" );
    }

}

void ppm_send_direct_p2 (int size_x, int size_y)
{
    int x, y, val;
    cc3_pixbuf_load ();
    printf ("P2\n%d %d\n255\n", size_x, size_y);
    for (y = 0; y < size_y; y++) {
        for (x = 0; x < size_x; x++) {
            val = cc3_pixbuf_read ();
            //if(val==0) { printf( "Damn, error in pixbuf_read()...\r\n" ); while(1); }
            printf ("%d ", cc3_g_current_pixel.channel[CC3_Y]);
        }
     printf( "\n" );
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

