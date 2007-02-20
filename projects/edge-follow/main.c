#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include <cc3_math.h>
#include <cc3_img_writer.h>
#include "polly.h"


//#define MMC_DEBUG

void draw_line_img (double b, double m, double distance, uint8_t conf);

/* simple hello world, showing features and compiling*/
int main (void)
{
  uint32_t last_time, val, i, x;
  char c;
  uint8_t *x_axis, *h, cnt, conf;
  polly_config_t p_config;
  cc3_image_t img;
  cc3_image_t tmp_img;
  cc3_image_t mask_img;
  cc3_pixel_t p, p_filter;
  cc3_pixel_t p_black, p_white;

  cc3_filesystem_init ();

  // configure uarts
  cc3_uart_init (0, CC3_UART_RATE_115200, CC3_UART_MODE_8N1,
                 CC3_UART_BINMODE_BINARY);
  // Make it so that stdout and stdin are not buffered
  val = setvbuf (stdout, NULL, _IONBF, 0);
  val = setvbuf (stdin, NULL, _IONBF, 0);

  cc3_camera_init ();

  cc3_camera_set_colorspace (CC3_COLORSPACE_RGB);
  cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_LOW);
  cc3_camera_set_auto_white_balance (false);
  cc3_camera_set_auto_exposure (true);
  cc3_pixbuf_frame_set_subsample (CC3_SUBSAMPLE_NEAREST, 2, 2);
  //cc3_pixbuf_frame_set_coi (CC3_CHANNEL_GREEN);
  cc3_pixbuf_frame_set_coi (CC3_CHANNEL_ALL);

  cc3_led_set_state (1, false);
  cc3_led_set_state (2, false);

  // load into pixbuf to initialize cc3_g_pixbuf_frame
  cc3_pixbuf_load();

  // sample wait command in ms 
  cc3_timer_wait_ms (1000);
  x_axis = malloc (cc3_g_pixbuf_frame.width);
  h = malloc (cc3_g_pixbuf_frame.width);

  p_config.color_thresh = 10;
  p_config.min_blob_size = 25;
  p_config.connectivity = 0;
  p_config.horizontal_edges = 0;
  p_config.vertical_edges = 1;
  p_config.blur = 1;
  p_config.histogram = malloc (cc3_g_pixbuf_frame.width);

  img.channels = 1;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = cc3_g_pixbuf_frame.height;      // image will hold just 1 row for scanline processing
  //img.pix = cc3_malloc_rows(1);
  img.pix = malloc (cc3_g_pixbuf_frame.height * cc3_g_pixbuf_frame.width);
  if (img.pix == NULL) {
    printf ("Not enough memory...\n");
    exit (0);
  }

  tmp_img.channels = 3;
  tmp_img.width = cc3_g_pixbuf_frame.width;
  tmp_img.height = 1;
  tmp_img.pix = cc3_malloc_rows (1);

  mask_img.channels = 1;
  mask_img.width = cc3_g_pixbuf_frame.width;
  mask_img.height = cc3_g_pixbuf_frame.height;
  mask_img.pix =
    malloc (cc3_g_pixbuf_frame.width * cc3_g_pixbuf_frame.height);

  p_black.channel[0] = 0;
  p_white.channel[0] = 255;
  while (1) {
    double distance;
    cc3_linear_reg_data_t reg_line;

    cc3_pixbuf_load ();

#ifdef MMC_DEBUG
    write_raw_fifo_ppm ();
    cc3_pixbuf_rewind ();
#endif

    for (i = 0; i < cc3_g_pixbuf_frame.height; i++) {
      cc3_pixbuf_read_rows (tmp_img.pix, tmp_img.height);
      for (x = 0; x < tmp_img.width; x++) {
        // process image using green channel
        cc3_get_pixel (&tmp_img, x, 0, &p);
        p_filter.channel[0] = p.channel[CC3_CHANNEL_GREEN];
        cc3_set_pixel (&img, x, i, &p_filter);

        // glare removal using red color channel
        if (p.channel[CC3_CHANNEL_RED] > 200 && p.channel[CC3_CHANNEL_GREEN] > 150)
          cc3_set_pixel (&mask_img, x, i, &p_black);
        else
          cc3_set_pixel (&mask_img, x, i, &p_white);
      }
    }

    polly (p_config, &img, &mask_img);
    // p_config.histogram gets filled with the return data

    // Prune away points on the histogram that are outliers so they don't
    // get added into the regression line.
    cnt = 0;
    for (i = 0; i < cc3_g_pixbuf_frame.width; i++) {
      if (p_config.histogram[i] > 0 && p_config.histogram[i] < 71) {
        h[cnt] = p_config.histogram[i];
        x_axis[cnt] = i;
        cnt++;
      }
    }

    printf ("pcnt = %d ", cnt);
    cc3_linear_reg (x_axis, h, cnt, &reg_line);

    printf ("b=%f ", reg_line.b);
    printf ("m=%f ", reg_line.m);
    printf ("r^2=%f ", reg_line.r_sqr);
    int32_t error;
    uint32_t total_error;
    double avg_error;
    error = 0;
    total_error = 0;
    for (i = 0; i < cnt; i++) {
      error = h[i] - (reg_line.m * (x_axis[i]) + reg_line.b);
      if (error < 0)
        error *= -1;
      total_error += error;
    }
    distance = reg_line.m * (cc3_g_pixbuf_frame.width / 2) + reg_line.b;
    printf ("distance = %f ", distance);
    avg_error = (double) total_error / (double) cnt;
    printf (" Abs Error = %d Avg Error = %f\n\r", total_error, avg_error);

    conf = 255;
    if (cnt < 30)
      conf = 100;
    if (avg_error > 3.0)
      conf = 75;
    //if(reg_line.r_sqr<.2) conf=100;       
//      draw_line_img(reg_line.b,reg_line.m,distance,conf);
    //  convert_histogram_to_ppm (&polly_img, config.histogram);

  }

  return 0;
}

void draw_line_img (double b, double m, double distance, uint8_t conf)
{
  cc3_image_t img;
  cc3_pixel_t p;
  uint32_t x;
  int32_t y;

  img.channels = 1;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = cc3_g_pixbuf_frame.height;
  img.pix = cc3_malloc_rows (cc3_g_pixbuf_frame.height);
  if (img.pix == NULL) {
    printf ("Not enough memory...\n");
    exit (0);
  }

  p.channel[0] = 0;
  for (y = 0; y < img.height; y++)
    for (x = 0; x < img.width; x++)
      cc3_set_pixel (&img, x, y, &p);

  p.channel[0] = conf;
  for (x = 0; x < img.width; x++) {
    y = (uint32_t) (m * x + b);
    if (y < 0 || y > img.height)
      y = 0;
    y = img.height - y;
    cc3_set_pixel (&img, x, y, &p);
  }
  cc3_img_write_file_create (&img);
  free (img.pix);
}
