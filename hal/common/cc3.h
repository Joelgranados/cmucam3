#ifndef CC3_H
#define CC3_H

#include "LPC2100.h"
#include "cc3_pin_defines.h"
#include "cc3_hal.h"
#include "serial.h"
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

typedef enum {
   CC3_DROP_2ND_GREEN,
   CC3_BUILD_2ND_PIXEL,
   CC3_BAYER
} cc3_pixel_mode_t;

typedef struct {
    uint16_t raw_width, raw_height;  // raw image width and height
    uint16_t width, height;  // subsampled and bound width and height
    uint16_t x0,y0,x1,y1;   // bounding box in frame
    uint16_t x_loc,y_loc;   // current position in frame
    uint8_t x_step, y_step;  // subsampling step
    cc3_channel_t coi;
    cc3_subsample_mode_t subsample_mode;
    cc3_pixel_mode_t pixel_mode;
} cc3_frame_t;



typedef struct {
    uint8_t channel[4];  // index with cc3_channel_t 
} cc3_pixel_t;   


typedef struct {
    uint16_t width, height;
    cc3_pixel_t* pix;
} cc3_image_t;


// Globals used by CMUcam functions
extern cc3_pixel_t cc3_g_current_pixel;   // global that gets updated with pixbuf calls
extern cc3_frame_t cc3_g_current_frame;   // global that keeps clip, stride

void cc3_pixbuf_load();
void _cc3_pixbuf_skip(uint32_t size);

void cc3_frame_default();
/**
 * Using the cc3_frame_t reads rows taking into account virtual window and subsampling. 
 */
int cc3_pixbuf_read_rows(cc3_pixel_t *mem, uint32_t width, uint32_t rows );

/**
 * loads cc3_g_current_pixel from fifo
 * Must adjust for channel, subframe, position in frame etc
 */
int cc3_pixbuf_read();                              

/**
 * The following are raw faster, pixel grab routines used by cc3_pixbuf_read_row(). 
 */
void _cc3_pixbuf_read_all(); 
void _cc3_pixbuf_read_all_3(); 
void _cc3_pixbuf_read_0(); 
void _cc3_pixbuf_read_1(); 
// No read_2() because it is just the second green channel
void _cc3_pixbuf_read_3(); 


/**
 * Rewinds the fifo 
 */
void cc3_pixbuf_rewind(void);                            
/**
 * Sets the region of interest in cc3_frame_t for virtual windowing. 
 */
int cc3_pixbuf_set_roi( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
/**
 * Sets the subsampling step and mode in cc3_frame_t. 
 */
int cc3_pixbuf_set_subsample( cc3_subsample_mode_t, uint8_t x_step, uint8_t y_step );

/**
 * Sets the channel of interest 1 or all
 */
int cc3_pixbuf_set_coi( cc3_channel_t chan );

int cc3_pixbuf_set_pixel_mode( cc3_pixel_mode_t mode);

void cc3_set_led(bool);

/**
 * 1) Enable Camera & FIFO Power
 * 2) Reset Camera
 * 3) call cc3_set functions for default state 
 */
int cc3_camera_init(void);
/**
 * Turn camera power off 
 * Turn fifo power off (may "cause picture to evaporate")
 */
void cc3_camera_kill(void);
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

void cc3_uart0_init(int32_t rate, uint8_t mode, uint8_t file_sel);
void cc3_uart1_init(int32_t rate, uint8_t mode, uint8_t file_sel);
void cc3_uart0_cr_lf(cc3_uart_cr_lf_t mode);
void cc3_uart1_cr_lf(cc3_uart_cr_lf_t mode);

uint32_t cc3_timer();

#endif
