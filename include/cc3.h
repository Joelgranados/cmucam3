/*
 * Copyright 2006-2007  Anthony Rowe and Adam Goode
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/** @file
 * The core of the cc3 system.
 */


#ifndef CC3_H
#define CC3_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/**
 * Mark a boolean expression as "likely".
 * This is sometimes useful for assisting the compiler in predicting branches.
 */
#define likely(x)   __builtin_expect(x,1)

/**
 * Mark a boolean expression as "unlikely".
 * This is sometimes useful for assisting the compiler in predicting branches.
 */
#define unlikely(x) __builtin_expect(x,0)

/**
 * Allowed resolutions for the camera device.
 */
typedef enum {
  CC3_LOW_RES = 0,  /**< QCIF */
  CC3_HIGH_RES = 1  /**< CIF */
} cc3_camera_resolution_t;

/**
 * Allowable channel values for channel of interest selection and
 * other functions.
 */
typedef enum {
  CC3_SINGLE = 0,  /**< Only channel in single-channel images */
  CC3_RED = 0,     /**< Red channel in RGB images */
  CC3_GREEN = 1,   /**< Green channel in RGB images */
  CC3_BLUE = 2,    /**< Blue channel in RGB images */
  CC3_Y = 0,       /**< Y channel in YCrCb images */
  CC3_CR = 1,      /**< Cr channel in YCrCb images */
  CC3_CB = 2,      /**< Cb channel in YCrCb images */
  CC3_ALL          /**< All channels in an image */
} cc3_channel_t;

/**
 * Colorspace selector.
 */
typedef enum {
  CC3_YCRCB = 0,   /**< YCrCb colorspace */
  CC3_RGB = 1      /**< RGB colorspace */
} cc3_colorspace_t;

/**
 * Subsampling modes.
 */
typedef enum {
  CC3_NEAREST,     /**< Nearest neighbor subsampling */
  CC3_MEAN,        /**< Mean of neighbors subsampling */
  CC3_RANDOM       /**< Random neighbor subsampling */
} cc3_subsample_mode_t;

/**
 * UART speeds. 115200 is the most common.
 * @sa cc3_uart_init().
 */
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


/**
 * UART modes.
 * 8N1 is the most common.
 * @sa cc3_uart_init().
 */
typedef enum {
  CC3_UART_MODE_8N1,   /**< 8 data bits, no parity, 1 stop bit */
  CC3_UART_MODE_7N1,   /**< 7 data bits, no parity, 1 stop bit */
  CC3_UART_MODE_8N2,   /**< 8 data bits, no parity, 2 stop bits */
  CC3_UART_MODE_7N2,   /**< 7 data bits, no parity, 2 stop bits */
  CC3_UART_MODE_8E1,   /**< 8 data bits, even parity, 1 stop bit */
  CC3_UART_MODE_7E1,   /**< 7 data bits, even parity, 1 stop bit */
  CC3_UART_MODE_8E2,   /**< 8 data bits, even parity, 2 stop bits */
  CC3_UART_MODE_7E2,   /**< 7 data bits, even parity, 2 stop bits */
  CC3_UART_MODE_8O1,   /**< 8 data bits, odd parity, 1 stop bit */
  CC3_UART_MODE_7O1,   /**< 7 data bits, odd parity, 1 stop bit */
  CC3_UART_MODE_8O2,   /**< 8 data bits, odd parity, 2 stop bits */
  CC3_UART_MODE_7O2    /**< 7 data bits, odd parity, 2 stop bits */
} cc3_uart_mode_t;

/**
 * UART binary/text mode selection values. @sa cc3_uart_init() for
 * how to use this.
 */
typedef enum {
  CC3_UART_BINMODE_BINARY,
  CC3_UART_BINMODE_TEXT,
} cc3_uart_binmode_t;

/**
 * Framebuffer definition.
 * @sa #cc3_g_pixbuf_frame for the main use of this definition.
 */
typedef struct {
  uint16_t raw_width;          /**< Native width */
  uint16_t raw_height;         /**< Native height */
  uint16_t x0;                 /**< Left horizontal clipping boundary */
  uint16_t y0;                 /**< Top vertical clipping boundary */
  uint16_t x1;                 /**< Right horizontal clipping boundary */
  uint16_t y1;                 /**< Bottom horizontal clipping boundary */
  uint16_t y_loc;              /**< Currently selected row */
  uint8_t x_step;              /**< Horizontal subsampling value */
  uint8_t y_step;              /**< Vertical subsampling value */
  cc3_channel_t coi;           /**< Channel of interest */
  cc3_subsample_mode_t subsample_mode;  /**< Subsampling mode */
  uint16_t width;              /**< Width of clipping region */
  uint16_t height;             /**< Height of clipping region */
  uint8_t channels;            /**< Number of channels */
  bool reset_on_next_load;     /**< True if the camera parameters have
				  changed */
} cc3_frame_t;

/**
 * Simple 3-channel pixel definition.
 */
typedef struct {
  uint8_t channel[3];          /**< Components of a single pixel */
} cc3_pixel_t;

/**
 * Current parameters for the internal pixbuf, should be
 * considered read only.
 * @sa cc3_pixbuf_frame_reset()
 */
extern cc3_frame_t cc3_g_pixbuf_frame;

/**
 * Allocate a number of rows of the correct size based on the values in
 * #cc3_g_pixbuf_frame.
 *
 * @param[in] rows The number of rows to allocate space for.
 * @return A pointer to allocated memory or NULL.
 */
uint8_t *cc3_malloc_rows (uint32_t rows);

/**
 * Take a picture with the camera and load it into the internal pixbuf.
 * If #cc3_g_pixbuf_frame has cc3_frame_t.reset_on_next_load set,
 * then cc3_g_pixbuf_frame is also reset with new values.
 */
void cc3_pixbuf_load (void);

/**
 * Reset the cc3_g_pixbuf_frame to default values.
 */
void cc3_pixbuf_frame_reset (void);

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
 * Sets the channel of interest 1 or all. Blah.
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
 * Sets the resolution, also updates cc3_g_pixbuf_frame width and height
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

/**
 * Awesome.
 */
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

void cc3_gpio_set_to_servo(uint8_t mask);
void cc3_gpio_set_to_input(uint8_t mask);
void cc3_gpio_set_to_output(uint8_t mask);
uint8_t cc3_gpio_set_pin(uint8_t pin);
uint8_t cc3_gpio_get_pin(uint8_t pin);


// call this if you want to use a filesystem
// without it, fopen won't really work
void cc3_filesystem_init (void);


#endif
