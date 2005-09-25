#ifndef CC3_H
#define CC3_H

#include "LPC2100.h"
#include "cc3_pin_defines.h"
#include "cc3_hal.h"
#include "inttypes.h"
#include <stdbool.h>
#include "interrupt.h" 


/*******************************************************************************
*  Initial CMUcam3 (cc3) data types and functions.
*
*******************************************************************************/



typedef enum  {
     CC3_LOW_RES=0,
     CC3_HIGH_RES=1
} cc3_camera_resolution_t;

typedef enum {
   CC3_GREEN=0,
   CC3_RED=1,
   CC3_GREEN2=2,
   CC3_BLUE=3,
   CC3_Y=0,
   CC3_CR=1,
   CC3_Y2=2,
   CC3_CB=3,
   CC3_ALL
} cc3_channel_t;

typedef enum {
   CC3_YCRCB=0,
   CC3_RGB=1
} cc3_colorspace_t ;

typedef enum {
   CC3_NEAREST,
   CC3_MEAN,
   CC3_RANDOM
} cc3_subsample_mode_t ;



typedef struct {
    uint16_t width, height;
    uint16_t x1,y1,x2,y2;
    uint8_t x_step, y_step;
} cc3_frame_t;

typedef struct {
    uint32_t channel[4];  // index with cc3_channel_t 
} cc3_pixel_t;    

typedef struct {
    uint16_t width, height;
    void* img;
} cc3_image_t;


// Globals used by CMUcam functions
extern cc3_pixel_t cc3_g_current_pixel;   // global that gets updated with pixbuf calls
extern cc3_frame_t cc3_g_current_frame;   // global that keeps clip, stride

void cc3_pixbuf_load();
void cc3_pixbuf_skip(uint32_t size);

/**
 * loads cc3_g_current_pixel from fifo
 */
void cc3_pixbuf_read();                              
/**
 * loads 3 bytes into cc3_g_current_pixel from fifo and skips second green. 
 */
void cc3_pixbuf_read3();                            
/**
 * Rewinds the fifo 
 */
void cc3_pixbuf_rewind();                            
/**
 * Using the cc3_frame_t reads rows taking into account virtual window and subsampling. 
 */
void cc3_pixbuf_read_rows( void* memory, uint32_t rows );
/**
 * Sets the region of interest in cc3_frame_t for virtual windowing. 
 */
int cc3_pixbuf_set_roi( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t x2);
/**
 * Sets the subsampling step and mode in cc3_frame_t. 
 */
int cc3_pixbuf_set_subsample( cc3_subsample_mode_t, uint8_t x_step, uint8_t y_step );

/**
 * Sets the channel of interest 1 or all
 */
int cc3_pixbuf_set_coi( cc3_channel_t chan );
void cc3_set_led(bool);

/**
 * 1) Enable Camera & FIFO Power
 * 2) Reset Camera
 * 3) call cc3_set functions for default state 
 */
int cc3_camera_init();
/**
 * Turn camera power off 
 * Turn fifo power off (may "cause picture to evaporate")
 */
void cc3_camera_kill();
/**
 * Sets the resolution, also updates cc3_g_current_frame width and height
 */
int cc3_set_resolution( cc3_camera_resolution_t );  
int cc3_set_colorspace( cc3_colorspace_t );
int cc3_set_framerate_divider( uint8_t rate_divider );
int cc3_set_auto_exposure( bool );
int cc3_set_auto_white_balance( bool );
int cc3_set_brightness( uint8_t level);
int cc3_set_contrast( uint8_t level);
int cc3_set_raw_register( uint8_t address, uint8_t value);
void cc3_io_init(int);


#endif
