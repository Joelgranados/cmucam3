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

  // init filesystem driver
  cc3_filesystem_init ();

  // configure uarts
  cc3_uart_init (0, CC3_UART_RATE_115200, CC3_UART_MODE_8N1,
                 CC3_UART_BINMODE_TEXT);
  // Make it so that stdout and stdin are not buffered
  val = setvbuf (stdout, NULL, _IONBF, 0);
  val = setvbuf (stdin, NULL, _IONBF, 0);

  cc3_camera_init ();

  cc3_camera_set_colorspace (CC3_COLORSPACE_RGB);
  cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_LOW);
  cc3_camera_set_auto_white_balance (true);
  cc3_camera_set_auto_exposure (true);

  printf ("Hello World...\n");

  cc3_led_set_state (0, false);
  cc3_led_set_state (1, false);
  cc3_led_set_state (2, false);

  // sample wait command in ms
  cc3_timer_wait_ms (1000);
  cc3_led_set_state (0, true);


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
  start_time = cc3_timer_get_current_ms ();
  while (!cc3_button_get_state ());
  cc3_led_set_state (1, true);
  // sample showing how to use timer
  printf ("It took you %dms to press the button\n",
          cc3_timer_get_current_ms () - start_time);



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

    if (val & 0x1)
      cc3_led_set_state (0, true);
    else
      cc3_led_set_state (0, false);
    if (val & 0x2)
      cc3_led_set_state (1, true);
    else
      cc3_led_set_state (1, false);
    if (val & 0x3)
      cc3_led_set_state (2, true);
    else
      cc3_led_set_state (2, false);
    if (val & 0x4)
      cc3_led_set_state (3, true);
    else
      cc3_led_set_state (3, false);
    val++;

    // This tells the camera to grab a new frame into the fifo and reset
    // any internal location information.
    cc3_pixbuf_frame_set_coi(CC3_CHANNEL_ALL);
    cc3_pixbuf_load ();


    // red search!

    // *** slow method for red search
    start_time = cc3_timer_get_current_ms();
    max_red = 0;
    my_x = 0;
    my_y = 0;
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
    end_time = cc3_timer_get_current_ms();

    printf ("Found max red value %d at %d, %d\n", max_red, my_x, my_y);
    printf (" cc3_get_pixel version took %d ms to complete\n",
	    end_time - start_time);

    // *** faster method for red search
    cc3_pixbuf_rewind();  // use exactly the same pixbuf contents
    start_time = cc3_timer_get_current_ms();
    max_red = 0;
    my_x = 0;
    my_y = 0;
    y = 0;
    while (cc3_pixbuf_read_rows (img.pix, 1)) {
      // read a row into the image picture memory from the camera
      for (uint16_t x = 0; x < img.width * 3; x+=3) {
	uint8_t red = ((uint8_t *) img.pix)[x + CC3_CHANNEL_RED];
        if (red > max_red) {
          max_red = red;
          my_x = x;
          my_y = y;
        }
      }
      y++;
    }
    my_x /= 3; // correct channel offset
    end_time = cc3_timer_get_current_ms();

    printf ("Found max red value %d at %d, %d\n", max_red, my_x, my_y);
    printf (" faster version took %d ms to complete\n",
	    end_time - start_time);

    // *** even faster method for red search
    cc3_pixbuf_rewind();  // use exactly the same pixbuf contents
    cc3_pixbuf_frame_set_coi(CC3_CHANNEL_RED);
    start_time = cc3_timer_get_current_ms();
    max_red = 0;
    my_x = 0;
    my_y = 0;
    y = 0;
    while (cc3_pixbuf_read_rows (img.pix, 1)) {
      // read a row into the image picture memory from the camera
      for (uint16_t x = 0; x < img.width; x++) {
	uint8_t red = ((uint8_t *) img.pix)[x];
        if (red > max_red) {
          max_red = red;
          my_x = x;
          my_y = y;
        }
      }
      y++;
    }
    end_time = cc3_timer_get_current_ms();

    printf ("Found max red value %d at %d, %d\n", max_red, my_x, my_y);
    printf (" even faster version took %d ms to complete\n",
	    end_time - start_time);

    printf("\n");
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
