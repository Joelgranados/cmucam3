/* ///////////////////////////////////////////////////////////////////
Surveillance Camera using CMUCam3

Created By:
Dhiraj Goel
ECE, CMU
Aug, 2006

Notes:
1. Uses low resolution images (176x144)
2. Fixed 2 stage thresholding
3. No filtering till now - can use median filtering to avoid speckle noise

////////////////////////////////////////////////////////////////////// */


#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <cc3.h>
#include <cc3_ilp.h>
#include "security_cam.h"

/* Constants */
static const uint8_t IMG_HEIGHT = 120;

/* Global variables to store the frames*/
static uint8_t *prev_img;
static uint8_t *curr_img;

/* Global thresholds */
static uint8_t GLOBAL_THRESH1 = 10;
static uint16_t GLOBAL_THRESH2;  // computed from width


// keep track of no. of frames saved
static uint16_t num_saved_frames = 0;

// keep track of no. of frames captured by the camera (but not necessarily saved)
static uint16_t num_captured_frames = 0;

/* array to store rows while transfering b/w fifo and ii */
static uint8_t *image_row;

static cc3_image_t buffer_img_row;

/* function to copy the image from the camera to the variable */
static void copy_frame_prev (int width)
{
  cc3_pixel_t pix_temp;

  // copy from camera memory, row by row
  uint8_t num_rows_read_from_fifo = 0;
  while (num_rows_read_from_fifo < IMG_HEIGHT) {
    cc3_pixbuf_read_rows (buffer_img_row.pix, buffer_img_row.height);

    for (uint8_t col_idx = 0; col_idx < width; col_idx++) {
      // get a pixel from the img row memory
      cc3_get_pixel (&buffer_img_row, col_idx, 0, &pix_temp);

      // copy the pixel intensity (gray scale value) into the image data structure
      curr_img[num_rows_read_from_fifo * width + col_idx] =
        pix_temp.channel[1];     //green channel ~ gray

      // copy the pixel intensity (gray scale value) into the image data structure
      prev_img[num_rows_read_from_fifo * width + col_idx] =
        pix_temp.channel[1];     //green channel ~ gray
    }

    // increment the row counter
    num_rows_read_from_fifo++;
  }
}

/* function to copy the frame from the camera and simultaneously compute the fame difference with the prev image */
static void copy_frame_n_compute_frame_diff (int width)
{
  cc3_pixel_t pix_temp;

  // copy from camera memory, row by row
  uint8_t num_rows_read_from_fifo = 0;
  while ((cc3_pixbuf_read_rows (buffer_img_row.pix, buffer_img_row.height)) &
         (num_rows_read_from_fifo < IMG_HEIGHT)) {
    // printf("iter: %d \r\n", num_rows_read_from_fifo);
    for (uint8_t col_idx = 0; col_idx < width; col_idx++) {
      // get a pixel from the img row memory
      cc3_get_pixel (&buffer_img_row, col_idx, 0, &pix_temp);

      // compute frame diff before storing the new pixel value
      prev_img[num_rows_read_from_fifo * width + col_idx] =
        abs (pix_temp.channel[1] -
             curr_img[num_rows_read_from_fifo * width + col_idx]);

      // copy the pixel intensity (gray scale value) into the image data structure
      curr_img[num_rows_read_from_fifo * width + col_idx] =
        pix_temp.channel[1];     //green channel ~ gray
    }

    // increment the counter for no. of rows read from fifo
    num_rows_read_from_fifo++;
  }
}

/* function to threshld the frane difference*/
static int8_t threshold_frame_diff (int width)
{
  uint16_t num_changed_pixels = 0;
  for (uint8_t row_idx = 0; row_idx < IMG_HEIGHT; row_idx++)
    for (uint8_t col_idx = 0; col_idx < width; col_idx++) {
      if (prev_img[row_idx * width + col_idx] > GLOBAL_THRESH1)
        if ((curr_img[row_idx * width + col_idx] >
             40) & (curr_img[row_idx * width + col_idx] < 210)) {
          num_changed_pixels++;
        }
    }

  // save the frame is this number exceeds the threshold
  if (num_changed_pixels > GLOBAL_THRESH2)
    return 1;
  else
    return 0;
}


/* function to save the current frame to MMC */
static void save_curr_frame (int width)
{
  // name of the curr image
  char img_name[50];

  FILE *fp;

  // open a file
#ifdef VIRTUAL_CAM
  sprintf (img_name, "img%04d.pgm", num_captured_frames);
#else
  sprintf (img_name, "c:/img%04d.pgm", num_captured_frames);
#endif
  printf ("Image Saved: %s \r\n", img_name);

  fp = fopen (img_name, "w");
  if (fp == NULL) {
    printf ("Error opening '%s'\r\n", img_name);
    perror("fopen");
    return;
  }

  // store the header
  fprintf (fp, "P2\n%d %d\n255\n", width, IMG_HEIGHT);

  // store the image data
  for (uint8_t row_idx = 0; row_idx < IMG_HEIGHT; row_idx++) {
    for (uint8_t col_idx = 0; col_idx < width; col_idx++) {
      fprintf (fp, "%d ", curr_img[row_idx * width + col_idx]);      // prev img contains frame diff
    }

    fprintf (fp, "\n");
  }

  // close the file
  fclose (fp);
}


int main (void)
{
  cc3_filesystem_init ();

  // configure uarts
  cc3_uart_init (0, CC3_UART_RATE_115200, CC3_UART_MODE_8N1,
                 CC3_UART_BINMODE_BINARY);

  cc3_camera_init ();

  cc3_camera_set_colorspace (CC3_COLORSPACE_RGB);
  cc3_camera_set_resolution (CC3_CAMERA_RESOLUTION_LOW);
  cc3_camera_set_auto_white_balance (true);
  cc3_camera_set_auto_exposure (true);

  // do things based on width
  const int width = cc3_g_pixbuf_frame.width;
  GLOBAL_THRESH2 = 0.2 * width * 132;
  prev_img = malloc(IMG_HEIGHT * width);
  curr_img = malloc(IMG_HEIGHT * width);
  image_row = cc3_malloc_rows(1);

  if (prev_img == NULL || curr_img == NULL || image_row == NULL) {
    printf("fatal error with malloc!\r\n");
    return -1;
  }


  printf ("Surveillance Camera...\r\n");

  // decalring buffer to read image from fifo
  buffer_img_row.channels = 3;
  buffer_img_row.width = width;
  buffer_img_row.height = 1;
  buffer_img_row.pix = image_row;

  // wait for stable
  cc3_timer_wait_ms (1000);
  cc3_led_set_state (0, false);
  cc3_led_set_state (1, false);
  cc3_led_set_state (2, false);
  cc3_led_set_state (3, false);

  cc3_led_set_state (0, true);           // indicate that the camera is ready

  // Grab the first frame and copy it as it is to the cc3_prev_img
  cc3_pixbuf_load ();
  copy_frame_prev (width);

  /* Start the while loop */
  while (1) {
    // load a new frame
    cc3_pixbuf_load ();

    // compute the frame diff (and store the new frame)
    copy_frame_n_compute_frame_diff (width);

    //save the frame if enough pixels have changed
    if (threshold_frame_diff (width) == 1) {
      cc3_led_set_state (1, true);          //blink the led as an indication

      save_curr_frame (width);       // save the frame

      // increment the counter for no. of saved frames
      num_saved_frames++;

      // wait for 1 sec before continuin with frame difference
      cc3_timer_wait_ms(1000);

      // load a new frame
      cc3_pixbuf_load ();

      // only copy the new frame and not compute frame difference
      copy_frame_prev (width);
    }

    cc3_led_set_state (1, false);

    // increment the total image captured counter
    num_captured_frames++;

  }

  // free the allocated memory
  free (prev_img);
  free (curr_img);
  free (image_row);

  // return
  return 0;
}
