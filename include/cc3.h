#ifndef CC3_H
#define CC3_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/*******************************************************************************
*  Initial CMUcam3 (cc3) data types and functions.
*
*******************************************************************************/

#define likely(x)   __builtin_expect(x,1)
#define unlikely(x) __builtin_expect(x,0)

typedef enum {
  CC3_LOW_RES = 0,
  CC3_HIGH_RES = 1
} cc3_camera_resolution_t;

typedef enum {
  CC3_RED = 0,
  CC3_GREEN = 1,
  CC3_BLUE = 2,
  CC3_Y = 0,
  CC3_CR = 1,
  CC3_CB = 2,
  CC3_ALL
} cc3_channel_t;

typedef enum {
  CC3_YCRCB = 0,
  CC3_RGB = 1
} cc3_colorspace_t;

typedef enum {
  CC3_NEAREST,
  CC3_MEAN,
  CC3_RANDOM
} cc3_subsample_mode_t;

typedef enum {
  CC3_UART_RATE_300 = 300,
  CC3_UART_RATE_600 = 600,
  CC3_UART_RATE_1200 = 1200,
  CC3_UART_RATE_2400 = 2400,
  CC3_UART_RATE_4800 = 4800,
  CC3_UART_RATE_9600 = 9600,
  CC3_UART_RATE_14400 = 14400,
  CC3_UART_RATE_19200 = 19200,
  CC3_UART_RATE_38400 = 38400,
  CC3_UART_RATE_57600 = 57600,
  CC3_UART_RATE_115200 = 115200,
  CC3_UART_RATE_230400 = 230400
} cc3_uart_rate_t;

typedef enum {
  CC3_UART_MODE_8N1,
  CC3_UART_MODE_7N1,
  CC3_UART_MODE_8N2,
  CC3_UART_MODE_7N2,
  CC3_UART_MODE_8E1,
  CC3_UART_MODE_7E1,
  CC3_UART_MODE_8E2,
  CC3_UART_MODE_7E2,
  CC3_UART_MODE_8O1,
  CC3_UART_MODE_7O1,
  CC3_UART_MODE_8O2,
  CC3_UART_MODE_7O2
} cc3_uart_mode_t;

typedef enum {
  CC3_UART_BINMODE_BINARY,
  CC3_UART_BINMODE_TEXT,
} cc3_uart_binmode_t;


typedef struct {
  uint16_t raw_width, raw_height;       // raw image width and height
  uint16_t x0, y0, x1, y1;      // bounding box in frame
  uint16_t y_loc;               // current position in frame
  uint8_t x_step, y_step;       // subsampling step
  cc3_channel_t coi;
  cc3_subsample_mode_t subsample_mode;
  uint16_t width, height;
  uint8_t channels;
} cc3_frame_t;



typedef struct {
  uint8_t channel[3];           // index with cc3_channel_t 
} cc3_pixel_t;


// Globals used by CMUcam functions
extern cc3_frame_t cc3_g_current_frame; // global that keeps clip, stride

uint8_t *cc3_malloc_rows (uint32_t rows);

void cc3_pixbuf_load (void);

void cc3_frame_default (void);
/**
 * Using the cc3_frame_t reads rows taking into account virtual window and subsampling. 
 */
int cc3_pixbuf_read_rows (void *mem, uint32_t rows);

/**
 * Rewinds the fifo 
 */
void cc3_pixbuf_rewind (void);
/**
 * Sets the region of interest in cc3_frame_t for virtual windowing. 
 */
int cc3_pixbuf_set_roi (int16_t x_0, int16_t y_0, int16_t x_1, int16_t y_1);
/**
 * Sets the subsampling step and mode in cc3_frame_t. 
 */
int cc3_pixbuf_set_subsample (cc3_subsample_mode_t, uint8_t x_step,
                              uint8_t y_step);

/**
 * Initialize the board. MUST be called for things to happen!
 */
void cc3_system_setup (void);

/**
 * Sets the channel of interest 1 or all
 */
int cc3_pixbuf_set_coi (cc3_channel_t chan);

void cc3_set_led (uint8_t select);
void cc3_clr_led (uint8_t select);
/**
 * 1) Enable Camera & FIFO Power
 * 2) Reset Camera
 * 3) call cc3_set functions for default state 
 */
int cc3_camera_init (void);
/**
 * Turn camera power off 
 * Turn fifo power off (may "cause picture to evaporate")
 */
void cc3_camera_kill (void);
/**
 * Sets the resolution, also updates cc3_g_current_frame width and height
 */
int cc3_set_resolution (cc3_camera_resolution_t);
int cc3_set_colorspace (cc3_colorspace_t);
int cc3_set_framerate_divider (uint8_t rate_divider);
int cc3_set_auto_exposure (bool);
int cc3_set_auto_white_balance (bool);
int cc3_set_brightness (uint8_t level);
int cc3_set_contrast (uint8_t level);
int cc3_set_raw_register (uint8_t address, uint8_t value);

uint8_t cc3_get_uart_count (void);
bool cc3_uart_init (uint8_t uart,
                    cc3_uart_rate_t rate,
                    cc3_uart_mode_t mode, cc3_uart_binmode_t binmode);
FILE *cc3_fopen_uart (uint8_t uart, const char *mode);
bool cc3_uart_has_data (uint8_t uart);


uint32_t cc3_timer (void);
void cc3_wait_ms (uint32_t delay);

bool cc3_read_button (void);



// Sets up the servo timers and begins servicing the servos
void cc3_servo_init (void);

// Set the servo mask
void cc3_servo_mask (uint32_t mask);

// User function to set a servo
uint8_t cc3_servo_set (uint8_t servo, uint32_t pos);

// User function to disable servos to conserve power
void cc3_servo_disable (void);




#endif
