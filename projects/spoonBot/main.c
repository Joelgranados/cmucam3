#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include "spoonBot.h"
#include <cc3_ilp.h>
#include <cc3.h>
#include <cc3_color_track.h>
#include <cc3_color_info.h>

void simple_track_color (cc3_track_pkt_t * t_pkt);
void simple_get_mean (cc3_color_info_pkt_t * s_pkt);

int main (void)
{
  cc3_track_pkt_t t_pkt;
  cc3_color_info_pkt_t s_pkt;
  spoonBot_calibrate_t my_cal;
  uint32_t x_mid, y_mid;
  uint32_t threshold, x0, y0, x1, y1;
  int32_t tmp, spoon_loc;

  cc3_uart_init (0, CC3_UART_RATE_115200, CC3_UART_MODE_8N1,
                 CC3_UART_BINMODE_TEXT);

  cc3_camera_init ();

  printf ("Starting up...\n");
  cc3_led_set_state (0, true);


  cc3_gpio_set_mode (0, CC3_GPIO_MODE_SERVO);
  cc3_gpio_set_mode (1, CC3_GPIO_MODE_SERVO);
  cc3_gpio_set_mode (2, CC3_GPIO_MODE_SERVO);
  cc3_gpio_set_mode (3, CC3_GPIO_MODE_OUTPUT);


  //printf ("Get calibration...\n");
  //spoonBot_get_calibration();

  // Before you use spoonbot, you must calibrate the servos.
  // See the spoonBot wiki for more details
  // 
  // SpoonBot Calibration Points
  //   Assume Left = Servo 0
  //   Assume Right = Servo 1
  //   Assume Spoon = Servo 2
  my_cal.left_mid = 71;
  my_cal.right_mid = 77;
  my_cal.spoon_down = 100;
  my_cal.spoon_mid = 202;
  my_cal.spoon_up = 255;
  my_cal.left_dir = 1;
  my_cal.right_dir = -1;
  my_cal.spoon_dir = 1;
  // Set the calibration points
  spoonBot_calibrate (my_cal);

  printf ("SpoonBot!\n");
  spoonBot_stop ();
  spoonBot_wait (100);
  printf ("SpoonBot Down\n");
  spoonBot_spoon_pos (-75);
  spoonBot_wait (100);
  printf ("SpoonBot Up\n");
  spoonBot_spoon_pos (75);
  spoonBot_wait (100);
  printf ("SpoonBot Mid\n");
  spoonBot_spoon_pos (0);
  spoonBot_wait (200);
  printf ("SpoonBot Right\n");
  spoonBot_right (30);
  spoonBot_wait (100);
  spoonBot_stop ();
  printf ("SpoonBot Left\n");
  spoonBot_left (30);
  spoonBot_wait (100);
  spoonBot_stop ();
  printf ("SpoonBot Done\n");


  cc3_camera_init ();
  cc3_camera_set_auto_exposure (true);
  cc3_camera_set_auto_white_balance (true);
  //cc3_camera_set_colorspace(CC3_COLORSPACE_YCRCB);
  cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_LOW);
  //cc3_pixbuf_frame_set_subsample(CC3_SUBSAMPLE_NEAREST, 2, 2);

  cc3_led_set_state (0, false);
  printf ("Waiting for image to stabilize\n");
  cc3_timer_wait_ms (2000);
  cc3_camera_set_auto_exposure (false);
  cc3_camera_set_auto_white_balance (false);

  cc3_led_set_state (0, true);
  printf ("Hold up colored object and press button...\n");
  while (cc3_button_get_state () == 0);
  printf ("Grabbing Color\n");

  // init pixbuf with width and height
  cc3_pixbuf_load ();


  threshold = 30;

  // set window to 1/2 size
  x0 = cc3_g_pixbuf_frame.x0 + cc3_g_pixbuf_frame.width / 4;
  x1 = cc3_g_pixbuf_frame.x1 - cc3_g_pixbuf_frame.width / 4;
  y0 = cc3_g_pixbuf_frame.y0 + cc3_g_pixbuf_frame.width / 4;
  y1 = cc3_g_pixbuf_frame.y1 - cc3_g_pixbuf_frame.width / 4;
  cc3_pixbuf_frame_set_roi (x0, y0, x1, y1);
  // call get mean
  simple_get_mean (&s_pkt);
  // set window back to full size
  x0 = 0;
  x1 = cc3_g_pixbuf_frame.raw_width;
  y0 = 0;
  y1 = cc3_g_pixbuf_frame.raw_height;
  cc3_pixbuf_frame_set_roi (x0, y0, x1, y1);
  // fill in parameters and call track color
  tmp = s_pkt.mean.channel[0] - threshold;
  if (tmp < 16)
    tmp = 16;
  if (tmp > 240)
    tmp = 240;
  t_pkt.lower_bound.channel[0] = tmp;
  tmp = s_pkt.mean.channel[0] + threshold;
  if (tmp < 16)
    tmp = 16;
  if (tmp > 240)
    tmp = 240;
  t_pkt.upper_bound.channel[0] = tmp;
  tmp = s_pkt.mean.channel[1] - threshold;
  if (tmp < 16)
    tmp = 16;
  if (tmp > 240)
    tmp = 240;
  t_pkt.lower_bound.channel[1] = tmp;
  tmp = s_pkt.mean.channel[1] + threshold;
  if (tmp < 16)
    tmp = 16;
  if (tmp > 240)
    tmp = 240;
  t_pkt.upper_bound.channel[1] = tmp;
  tmp = s_pkt.mean.channel[2] - threshold;
  if (tmp < 16)
    tmp = 16;
  if (tmp > 240)
    tmp = 240;
  t_pkt.lower_bound.channel[2] = tmp;
  tmp = s_pkt.mean.channel[2] + threshold;
  if (tmp < 16)
    tmp = 16;
  if (tmp > 240)
    tmp = 240;
  t_pkt.upper_bound.channel[2] = tmp;


  printf ("Got color min=[%d,%d,%d] max=[%d,%d,%d]\n",
          t_pkt.lower_bound.channel[0], t_pkt.lower_bound.channel[1],
          t_pkt.lower_bound.channel[2], t_pkt.upper_bound.channel[0],
          t_pkt.upper_bound.channel[1], t_pkt.upper_bound.channel[2]);


  // Load in your tracking parameters
  // t_pkt.lower_bound.channel[CC3_CHANNEL_RED] = 200;
  // t_pkt.upper_bound.channel[CC3_CHANNEL_RED] = 255;
  // t_pkt.lower_bound.channel[CC3_CHANNEL_GREEN] = 0;
  // t_pkt.upper_bound.channel[CC3_CHANNEL_GREEN] = 110;
  // t_pkt.lower_bound.channel[CC3_CHANNEL_BLUE] = 0;
  // t_pkt.upper_bound.channel[CC3_CHANNEL_BLUE] = 20; 

  t_pkt.noise_filter = 4;
  t_pkt.track_invert = false;

  x_mid = cc3_g_pixbuf_frame.width / 2;
  y_mid = cc3_g_pixbuf_frame.height / 2;
  spoon_loc = 0;
  while (true) {
    uint8_t track_flag;
    simple_track_color (&t_pkt);
    track_flag = 0;
    if (t_pkt.int_density > 10 && t_pkt.num_pixels > 100) {
      printf
        ("centroid = %d,%d bounding box = %d,%d,%d,%d num pix= %d density = %d\n",
         t_pkt.centroid_x, t_pkt.centroid_y, t_pkt.x0, t_pkt.y0, t_pkt.x1,
         t_pkt.y1, t_pkt.num_pixels, t_pkt.int_density);

      if (t_pkt.centroid_x > (x_mid + 20)) {
        track_flag = 1;
        spoonBot_right (10);
      }
      else if (t_pkt.centroid_x > (x_mid + 5)) {
        track_flag = 1;
        spoonBot_right (3);
      }

      if (t_pkt.centroid_x < (x_mid - 20)) {
        track_flag = 1;
        spoonBot_left (10);
      }
      else if (t_pkt.centroid_x < (x_mid - 5)) {
        track_flag = 1;
        spoonBot_left (3);
      }

      if (t_pkt.centroid_y > (y_mid + 10)) {
        spoon_loc -= 3;
        track_flag = 1;
      }

      if (t_pkt.centroid_y < (y_mid - 10)) {
        spoon_loc += 3;
        track_flag = 1;
      }
      if (spoon_loc > 100)
        spoon_loc = 100;
      if (spoon_loc < -100)
        spoon_loc = -100;
      spoonBot_spoon_pos (spoon_loc);
    }
    if (track_flag == 0)
      spoonBot_stop ();
  }


  return 0;
}


void simple_track_color (cc3_track_pkt_t * t_pkt)
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
  if (cc3_track_color_scanline_start (t_pkt) != 0) {
    while (cc3_pixbuf_read_rows (img.pix, 1)) {
      // This does the HSV conversion 
      // cc3_rgb2hsv_row(img.pix,img.width);
      cc3_track_color_scanline (&img, t_pkt);
    }
  }
  cc3_track_color_scanline_finish (t_pkt);

  free (img.pix);
  return;
}


void simple_get_mean (cc3_color_info_pkt_t * s_pkt)
{
  cc3_image_t img;
  img.channels = 3;
  img.width = cc3_g_pixbuf_frame.width;
  img.height = 1;               // image will hold just 1 row for scanline processing
  img.pix = malloc (3 * img.width);

  cc3_pixbuf_load ();
  if (cc3_color_info_scanline_start (s_pkt) != 0) {
    while (cc3_pixbuf_read_rows (img.pix, 1)) {
      cc3_color_info_scanline (&img, s_pkt);
    }
    cc3_color_info_scanline_finish (s_pkt);
  }
  free (img.pix);
}
