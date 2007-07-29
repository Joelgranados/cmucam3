#include <stdio.h>
#include <stdlib.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include <cc3_histogram.h>

void simple_get_histogram (cc3_histogram_pkt_t * h_pkt);

int main (void)
{
  cc3_histogram_pkt_t my_hist;
  uint32_t i;

  cc3_uart_init (0,
                 CC3_UART_RATE_115200,
                 CC3_UART_MODE_8N1, CC3_UART_BINMODE_TEXT);

  cc3_camera_init ();


  //cc3_camera_set_colorspace(CC3_COLORSPACE_YCRCB);
  cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_LOW);
  //cc3_pixbuf_frame_set_subsample(CC3_SUBSAMPLE_NEAREST, 2, 2);

  // init pixbuf with width and height
  // When using the virtual-cam, note that this will skip the first image
  cc3_pixbuf_load ();

  my_hist.channel = CC3_CHANNEL_GREEN;
  my_hist.bins = 24;
  my_hist.hist = malloc (my_hist.bins * sizeof (uint32_t));

  while (true) {
    // Grab an image and take a histogram of it
    simple_get_histogram (&my_hist);

    // Print the histogram out on the screen
    printf ("hist: ");
    for (i = 0; i < my_hist.bins; i++)
      printf ("%d ", my_hist.hist[i]);
    printf ("\n");
  }
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
