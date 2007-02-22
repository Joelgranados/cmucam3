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

// keep track of no. of frames saved
static uint16_t num_saved_frames = 0;

// keep track of no. of frames captured by the camera (but not necessarily saved)
static uint16_t num_captured_frames = 0;

/* array to store rows while transfering b/w fifo and ii */
static uint8_t image_row[CC3_LO_RES_WIDTH * 3];

cc3_image_t buffer_img_row;

FILE *fin;

/* function to copy the image from the camera to the variable */
void copy_frame_prev ()
{
  cc3_pixel_t pix_temp;

  // copy from camera memory, row by row
  uint8_t num_rows_read_from_fifo = 0;
  while (num_rows_read_from_fifo < CC3_IMG_HEIGHT) {
    cc3_pixbuf_read_rows (buffer_img_row.pix, buffer_img_row.height);

    for (uint8_t col_idx = 0; col_idx < CC3_IMG_WIDTH; col_idx++) {
      // get a pixel from the img row memory
      cc3_get_pixel (&buffer_img_row, col_idx, 0, &pix_temp);

      // copy the pixel intensity (gray scale value) into the image data structure
      cc3_curr_img[num_rows_read_from_fifo][col_idx] = pix_temp.channel[1];     //green channel ~ gray

      // copy the pixel intensity (gray scale value) into the image data structure
      cc3_prev_img[num_rows_read_from_fifo][col_idx] = pix_temp.channel[1];     //green channel ~ gray
    }

    // increment the row counter
    num_rows_read_from_fifo++;
  }
}

/* function to copy the frame from the camera and simultaneously compute the fame difference with the prev image */
void copy_frame_n_compute_frame_diff ()
{
  cc3_pixel_t pix_temp;

  // copy from camera memory, row by row
  uint8_t num_rows_read_from_fifo = 0;
  while ((cc3_pixbuf_read_rows (buffer_img_row.pix, buffer_img_row.height)) &
         (num_rows_read_from_fifo < CC3_IMG_HEIGHT)) {
    // printf("iter: %d \n\r", num_rows_read_from_fifo);
    for (uint8_t col_idx = 0; col_idx < CC3_IMG_WIDTH; col_idx++) {
      // get a pixel from the img row memory
      cc3_get_pixel (&buffer_img_row, col_idx, 0, &pix_temp);

      // compute frame diff before storing the new pixel value
      cc3_prev_img[num_rows_read_from_fifo][col_idx] =
        abs (pix_temp.channel[1] -
             cc3_curr_img[num_rows_read_from_fifo][col_idx]);

      // copy the pixel intensity (gray scale value) into the image data structure
      cc3_curr_img[num_rows_read_from_fifo][col_idx] = pix_temp.channel[1];     //green channel ~ gray
    }

    // increment the counter for no. of rows read from fifo
    num_rows_read_from_fifo++;
  }
}

/* function to threshld the frane difference*/
int8_t threshold_frame_diff ()
{
  uint16_t num_changed_pixels = 0;
  for (uint8_t row_idx = 0; row_idx < CC3_IMG_HEIGHT; row_idx++)
    for (uint8_t col_idx = 0; col_idx < CC3_IMG_WIDTH; col_idx++) {
      if (cc3_prev_img[row_idx][col_idx] > CC3_GLOBAL_THRESH1)
        if ((cc3_curr_img[row_idx][col_idx] >
             40) & (cc3_curr_img[row_idx][col_idx] < 210)) {
          num_changed_pixels++;
        }
    }

  // save the frame is this number exceeds the threshold
  if (num_changed_pixels > CC3_GLOBAL_THRESH2)
    return 1;
  else
    return 0;
}


/* function to save the current frame to MMC */
void save_curr_frame ()
{
  // name of the curr image
  char img_name[50];

  FILE *fp;

  // open a file
  sprintf (img_name, "%s%04d%s", "c:/img", num_captured_frames, ".pgm");
  printf ("% Image Saved: s \n\r", img_name);

  fp = fopen (img_name, "w");
  if (fp == NULL) {
    printf ("%s %s\n\r", "Error Opening: ", img_name);
  }

  // store the header
  fprintf (fp, "P2\n%d %d\n255\n", CC3_IMG_WIDTH, CC3_IMG_HEIGHT);

  // store the image data
  for (uint8_t row_idx = 0; row_idx < CC3_IMG_HEIGHT; row_idx++) {
    for (uint8_t col_idx = 0; col_idx < CC3_IMG_WIDTH; col_idx++) {
      fprintf (fp, "%d ", cc3_curr_img[row_idx][col_idx]);      // prev img contains frame diff
    }

    fprintf (fp, "\n");
  }

  // close the file
  fclose (fp);
}


/* simple hello world, showing features and compiling*/
int main (void)
{
  uint32_t start_time;

  cc3_filesystem_init ();
  // configure uarts
  cc3_uart_init (0, CC3_UART_RATE_115200, CC3_UART_MODE_8N1,
                 CC3_UART_BINMODE_BINARY);

  cc3_camera_init ();

  cc3_set_colorspace (CC3_RGB);
  cc3_set_resolution (CC3_LOW_RES);
  cc3_set_auto_white_balance (true);
  cc3_set_auto_exposure (true);

  printf ("Surveillance Camera...\n\r");

  // decalring buffer to read image from fifo
  buffer_img_row.channels = 3;
  buffer_img_row.width = CC3_IMG_WIDTH;
  buffer_img_row.height = 1;
  buffer_img_row.pix = &image_row;

  // sample wait command in ms
  cc3_wait_ms (1000);
  cc3_clr_led (0);
  cc3_clr_led (1);
  cc3_clr_led (2);

  cc3_set_led (0);              // indicate that the camera is ready

  // Grab the first frame and copy it as it is to the cc3_prev_img
  cc3_pixbuf_load ();
  copy_frame_prev ();

  /* Start the while loop */
  while (1) {
    // load a new frame
    cc3_pixbuf_load ();

    // compute the frame diff (and store the new frame)
    copy_frame_n_compute_frame_diff ();

    //save the frame if enough pixels have changed
    if (threshold_frame_diff () == 1) {
      cc3_set_led (1);          //blink the led as an indication

      save_curr_frame ();       // save the frame

      // increment the counter for no. of saved frames
      num_saved_frames++;

      // wait for 1 sec before continuin with frame difference
      start_time = cc3_timer ();
      while ((cc3_timer () - start_time) < 1000) {;
      }

      // load a new frame
      cc3_pixbuf_load ();

      // only copy the new frame and not compute frame difference
      copy_frame_prev ();
    }

    cc3_clr_led (1);

    // increment the total image captured counter
    num_captured_frames++;

  }

  // free the allocated memory
  free (buffer_img_row.pix);

  // return
  return 0;
}
