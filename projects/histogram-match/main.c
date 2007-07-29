#include <stdio.h>
#include <stdlib.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include <cc3_histogram.h>

#define TEMPLATE_IMGS     3
#define DETECT_THRESH	  2000

void simple_get_histogram (cc3_histogram_pkt_t * h_pkt);
uint32_t histogram_subtract (cc3_histogram_pkt_t * h1,
                             cc3_histogram_pkt_t * h2);

int main (void)
{
  cc3_histogram_pkt_t my_hist;
  cc3_histogram_pkt_t train_hist[TEMPLATE_IMGS];
  uint32_t i;

  cc3_uart_init (0,
                 CC3_UART_RATE_115200,
                 CC3_UART_MODE_8N1, CC3_UART_BINMODE_TEXT);

  cc3_camera_init ();


  //cc3_camera_set_colorspace(CC3_COLORSPACE_YCRCB);
  cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_HIGH);
  //cc3_pixbuf_frame_set_subsample(CC3_SUBSAMPLE_NEAREST, 2, 2);

  // Set a region on the image where we take a histogram.
  // This could be the entire image, or just a section.
  cc3_pixbuf_frame_set_roi (160, 50, 200, 240);

  // init pixbuf with width and height
  // When using the virtual-cam, note that this will skip the first image
  cc3_pixbuf_load ();

  my_hist.channel = CC3_CHANNEL_GREEN;
  my_hist.bins = 24;
  my_hist.hist = malloc (my_hist.bins * sizeof (uint32_t));

  cc3_led_set_state (0, 0);
  cc3_led_set_state (1, 0);
  cc3_led_set_state (2, 0);
  // Read in the next TEMPLATE_IMGS number of images to build templates
  for (i = 0; i < TEMPLATE_IMGS; i++) {
    cc3_led_set_state (i, 1);
    printf( "Press button to train image %d...\n",i );
    // Wait for button press to capture template image
    while (!cc3_button_get_state ());
    cc3_led_set_state (i, 0);
    train_hist[i].channel = CC3_CHANNEL_GREEN;
    train_hist[i].bins = 24;
    train_hist[i].hist = malloc (train_hist[i].bins * sizeof (uint32_t));
    simple_get_histogram (&train_hist[i]);
    printf( "Image loaded...\n" );
    cc3_timer_wait_ms(1000);
  }
  // Turn off LEDs when training done
  cc3_led_set_state (0, 0);
  cc3_led_set_state (1, 0);
  cc3_led_set_state (2, 0);

  while (true) {
    uint32_t diff, min;
    int8_t template;

    // Grab an image and take a histogram of it
    simple_get_histogram (&my_hist);

    // Print the histogram out on the screen
    printf ("hist: ");
    for (i = 0; i < my_hist.bins; i++)
      printf ("%d ", my_hist.hist[i]);
    printf ("\n");

    // Look for the min absolute difference between the new image and the templates
    min = 0xFFFFFFFF;
    template = -1;
    for (i = 0; i < TEMPLATE_IMGS; i++) {
      diff = histogram_subtract (&my_hist, &train_hist[i]);
      if (diff < min) {
        min = diff;
        template = i;
      }
    }

      cc3_led_set_state (0, 0);
      cc3_led_set_state (1, 0);
      cc3_led_set_state (2, 0);
    if (min < DETECT_THRESH) {
      cc3_led_set_state (template, 1);
      printf ("Image matched template %d with value %d\n", template, min);
    }
    else {
      printf ("NO IMAGE MATCHED: closest template %d with value %d\n",
              template, min);
    }
  }

}

uint32_t histogram_subtract (cc3_histogram_pkt_t * h1,
                             cc3_histogram_pkt_t * h2)
{
  uint32_t diff;
  uint32_t i;

  if (h1->bins != h2->bins) {
    printf ("histogram_subtract error:  bin size mismatch\n");
    return 0;
  }

  diff = 0;
  for (i = 0; i < h1->bins; i++) {
    if (h1->hist[i] > h2->hist[i])
      diff += h1->hist[i] - h2->hist[i];
    else
      diff += h2->hist[i] - h1->hist[i];
  }
  return diff;
}

void simple_get_histogram (cc3_histogram_pkt_t * h_pkt)
{
  cc3_image_t img;

  img.channels = 3;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = 1;               // image will hold just 1 row for scanline processing
  img.pix = cc3_malloc_rows (1);
  if (img.pix == NULL) {
    return;
  }

  cc3_pixbuf_load ();
  if (cc3_histogram_scanline_start (h_pkt) != 0) {
    while (cc3_pixbuf_read_rows (img.pix, 1)) {
      // This does the HSV conversion 
      // cc3_rgb2hsv_row(img.pix,img.width);
      cc3_histogram_scanline (&img, h_pkt);
    }
  }
  cc3_histogram_scanline_finish (h_pkt);

  free (img.pix);
  return;
}
