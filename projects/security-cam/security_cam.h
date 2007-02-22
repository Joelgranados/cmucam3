// header file for surveillance camera application


#ifndef SURVCAM_H
#define SURVCAM_H

/* ----------- functions -----------------*/

/* function to copy the image from the camera to the variable */
static void copy_frame_prev(int width);

/* function to copy the frame from the camera and simultaneously compute the fame difference with the prev image */
static void copy_frame_n_compute_frame_diff(int width);

/* function to threshld the frane difference */
static int8_t threshold_frame_diff(int width);

/* function to save the current frame to MMC */
static void save_curr_frame(int width);

#endif
