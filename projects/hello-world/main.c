#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>


/* simple hello world, showing features and compiling*/
int main (void)
{
  uint32_t start_time, end_time, val;
  char c;
  FILE *fp;
  cc3_image_t img;

  // setup system
  cc3_system_setup ();

  // init filesystem driver
  cc3_filesystem_init ();

  // configure uarts
  cc3_uart_init (0, CC3_UART_RATE_115200, CC3_UART_MODE_8N1,
                 CC3_UART_BINMODE_TEXT);
  // Make it so that stdout and stdin are not buffered
  val = setvbuf (stdout, NULL, _IONBF, 0);
  val = setvbuf (stdin, NULL, _IONBF, 0);

  cc3_camera_init ();

  cc3_set_colorspace (CC3_COLORSPACE_RGB);
  cc3_set_resolution (CC3_RES_LOW);
  cc3_set_auto_white_balance (true);
  cc3_set_auto_exposure (true);

  printf ("Hello World...\n");

  cc3_clr_led (0);
  cc3_clr_led (1);
  cc3_clr_led (2);

  // sample wait command in ms
  cc3_wait_ms (1000);
  cc3_set_led (0);


  // sample showing how to write to the MMC card
  printf ("Type y to test MMC card, type n if you do not have the card\n");
  c = getchar ();
  if (c == 'y' || c == 'Y') {
    int result;
    printf ("\nMMC test...\n");
    fp = fopen ("c:/test.txt", "w");
    if (fp == NULL) {
      perror ("fopen failed");
    }
    fprintf (fp, "This will be written to the MMC...\n");

    result = fclose (fp);
    if (result == EOF) {
      perror ("fclose failed");
    }
    printf ("A string was written to test.txt on the mmc card.\n");
  }

  // sample showing how to read button
  printf ("push button on camera back to continue\n");
  start_time = cc3_get_current_ms ();
  while (!cc3_read_button ());
  cc3_set_led (1);
  // sample showing how to use timer
  printf ("It took you %dms to press the button\n",
          cc3_get_current_ms () - start_time);



  // setup an image structure
  cc3_pixbuf_load ();
  img.channels = 3;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = 1;               // image will hold just 1 row for scanline processing
  img.pix = cc3_malloc_rows (1);

  printf ("Now we will use image data...\n");
  val = 0;
  /*
   * Track the brightest red spot on the image
   */
  while (1) {
    int y;
    uint16_t my_x, my_y;
    uint8_t max_red;
    cc3_pixel_t my_pix;
    max_red = 0;
    my_x = 0;
    my_y = 0;

    if (val & 0x1)
      cc3_set_led (0);
    else
      cc3_clr_led (0);
    if (val & 0x2)
      cc3_set_led (1);
    else
      cc3_clr_led (1);
    if (val & 0x3)
      cc3_set_led (2);
    else
      cc3_clr_led (2);
    if (val & 0x4)
      cc3_set_led (3);
    else
      cc3_clr_led (3);
    val++;

    // This tells the camera to grab a new frame into the fifo and reset
    // any internal location information.
    cc3_pixbuf_load ();

    start_time = cc3_get_current_ms();
    y = 0;
    while (cc3_pixbuf_read_rows (img.pix, 1)) {
      // read a row into the image picture memory from the camera
      for (uint16_t x = 0; x < img.width; x++) {
        // get a pixel from the img row memory
        cc3_get_pixel (&img, x, 0, &my_pix);
        if (my_pix.channel[CC3_CHANNEL_RED] > max_red) {
          max_red = my_pix.channel[CC3_CHANNEL_RED];
          my_x = x;
          my_y = y;
        }
      }
      y++;
    }
    end_time = cc3_get_current_ms();

    printf ("Found max red value %d at %d, %d\n", max_red, my_x, my_y);
    printf (" cc3_get_pixel version took %d ms to complete\n",
	    end_time - start_time);

    // sample non-blocking serial routine
    if (!cc3_uart_has_data (0))
      break;
  }
  free (img.pix);               // don't forget to free!
  printf ("You pressed %c to escape\n", fgetc (stdin));

  // stdio actually works...
  printf ("Type in a number followed by return to test scanf: ");
  scanf ("%d", &val);
  printf ("You typed %d\n", val);

  printf ("Good work, now try something on your own...\n");
  while (1);

  return 0;
}
