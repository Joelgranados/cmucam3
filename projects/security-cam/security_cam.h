// header file for surveillance camera application


#ifndef SURVCAM_H
#define SURVCAM_H

/* -------------- data -------------------*/

/* Constants */
static const uint8_t CC3_IMG_HEIGHT = 120;
static const uint8_t CC3_IMG_WIDTH = 176;

/* Global variables to store the frames*/
static uint8_t cc3_prev_img[120][176];
static uint8_t cc3_curr_img[120][176];

/* Global thresholds */
static uint8_t CC3_GLOBAL_THRESH1 = 10;
static uint16_t CC3_GLOBAL_THRESH2 = 0.2*176*132;
/* ----------- functions -----------------*/

/* function to copy the image from the camera to the variable */
void copy_frame_prev(void);

/* function to copy the frame from the camera and simultaneously compute the fame difference with the prev image */
void copy_frame_n_compute_frame_diff(void);

/* function to threshld the frane difference */
int8_t threshold_frame_diff(void);

/* function to save the current frame to MMC */
void save_curr_frame(void);

#endif
