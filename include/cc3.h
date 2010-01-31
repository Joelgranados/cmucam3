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

/**
 * @file
 * The core of the cc3 system.
 */


#ifndef CC3_H
#define CC3_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/**
 * Major version number of API. This is incremented when
 * backwards-incompatible changes are made in a release.
 */
#define CC3_API_MAJOR_VERSION  1

/**
 * Minor version number of API. This is incremented when compatible
 * new features are added in a release.
 */
#define CC3_API_MINOR_VERSION  0


/**
 * Allowed resolutions for the camera device.
 */
typedef enum {
  CC3_CAMERA_RESOLUTION_LOW = 0,  /**< Low resolution */
  CC3_CAMERA_RESOLUTION_HIGH = 1  /**< High resolution */
} cc3_camera_resolution_t;

/**
 * Allowable channel values for channel of interest selection and
 * other functions.
 */
typedef enum {
  CC3_CHANNEL_SINGLE = 0,  /**< Only channel in single-channel images */
  CC3_CHANNEL_RED = 0,     /**< Red channel in RGB images */
  CC3_CHANNEL_GREEN = 1,   /**< Green channel in RGB images */
  CC3_CHANNEL_BLUE = 2,    /**< Blue channel in RGB images */
  CC3_CHANNEL_Y = 0,       /**< Y channel in YCrCb images */
  CC3_CHANNEL_CR = 1,      /**< Cr channel in YCrCb images */
  CC3_CHANNEL_CB = 2,      /**< Cb channel in YCrCb images */
  CC3_CHANNEL_HUE = 0,       /**< Hue channel in HSV images */
  CC3_CHANNEL_SAT = 1,      /**< Sat channel in HSV images */
  CC3_CHANNEL_VAL = 2,      /**< Val channel in HSV images */
  CC3_CHANNEL_ALL          /**< All channels in an image */
} cc3_channel_t;

/**
 * Colorspace selector.
 */
typedef enum {
  CC3_COLORSPACE_YCRCB = 0,   /**< YCrCb colorspace */
  CC3_COLORSPACE_RGB = 1,      /**< RGB colorspace */
  CC3_COLORSPACE_HSV = 2,      /**< HSV colorspace */
  CC3_COLORSPACE_MONOCHROME = 3      /**< Monochrome colorspace */
} cc3_colorspace_t;

/**
 * Subsampling modes.
 */
typedef enum {
  CC3_SUBSAMPLE_NEAREST,     /**< Nearest neighbor subsampling */
  CC3_SUBSAMPLE_MEAN,        /**< Mean of neighbors subsampling */
  CC3_SUBSAMPLE_RANDOM       /**< Random neighbor subsampling */
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
  CC3_UART_RATE_230400 = 230400,
  CC3_UART_RATE_921600 = 921600
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
 * where to use this.
 */
typedef enum {
  CC3_UART_BINMODE_BINARY,   /**< Do not alter serial data in any way */
  CC3_UART_BINMODE_TEXT,     /**< Read CR as LF, write LF as CR */
} cc3_uart_binmode_t;

/**
 * GPIO configuration values.
 * @sa cc3_gpio_set_mode() and cc3_gpio_get_mode().
 */
typedef enum {
  CC3_GPIO_MODE_INPUT,       /**< Set pin for input */
  CC3_GPIO_MODE_OUTPUT,      /**< Set pin for output */
  CC3_GPIO_MODE_SERVO,       /**< Set pin for servo output */
} cc3_gpio_mode_t;

/**
 * Frame structure.
 * @sa #cc3_g_pixbuf_frame for the main use of this structure.
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
} cc3_frame_t;

/**
 * Simple 3-channel pixel structure.
 */
typedef struct {
  uint8_t channel[3];          /**< Components of a single pixel */
} cc3_pixel_t;

/**
 * Current parameters for the internal pixbuf, should be
 * considered read only.
 */
extern cc3_frame_t cc3_g_pixbuf_frame;

/**
 * Initialize camera hardware. This resets all camera and pixbuf parameters.
 *
 * @return \a true if successful.
 */
bool cc3_camera_init (void);

/**
 * Initialize the filesystem drivers. Without this call, the FAT filesystem
 * will not be enabled, saving significant code space.
 */
void cc3_filesystem_init (void);

/**
 * Take a picture with the camera and load it into the internal pixbuf.
 */
void cc3_pixbuf_load (void);

/**
 * Use malloc() to allocate a number of rows of the correct size based
 * on the values in #cc3_g_pixbuf_frame. You must manually use free()
 * to deallocate this memory when you are done.
 *
 * @sa cc3_pixbuf_read_rows()
 *
 * @param[in] rows The number of rows to allocate space for.
 * @return A pointer to allocated memory or \a NULL if error.
 */
uint8_t *cc3_malloc_rows (uint32_t rows);

/**
 * Do a row-by-row copy from the pixbuf into a block of memory.
 * This function takes into account all of the parameters listed in
 * #cc3_g_pixbuf_frame.
 *
 * @param[out] mem The memory to write the rows to.
 * @param[in] rows The number of rows to copy.
 * @return Number of rows copied.
 */
int cc3_pixbuf_read_rows (void *mem, uint32_t rows);

/**
 * Rewind the pixbuf to the beginning without changing the
 * image data stored in it. This function allows you to seek to
 * the beginning of the pixbuf without calling cc3_pixbuf_load().
 */
void cc3_pixbuf_rewind (void);

/**
 * Set the region of interest in #cc3_g_pixbuf_frame for virtual windowing.
 *
 * @param[in] x_0 Left horizontal clipping boundary.
 * @param[in] y_0 Top vertical clipping boundary.
 * @param[in] x_1 Right horizontal clipping boundary.
 * @param[in] y_1 Bottom vertical clipping boundary.
 * @return \a true if successful.
 */
bool cc3_pixbuf_frame_set_roi (int16_t x_0,
                               int16_t y_0,
                               int16_t x_1,
                               int16_t y_1);

/**
 * Set the subsample steps and mode in #cc3_g_pixbuf_frame.
 *
 * @param[in] mode Subsample mode.
 * @param[in] x_step Horizontal step. Must be either 1 or an even value.
 * @param[in] y_step Vertical step.
 * @return \a true if completely successful. Some settings may be partially
 *         applied even if \a false.
 */
bool cc3_pixbuf_frame_set_subsample (cc3_subsample_mode_t mode,
                                     uint8_t x_step,
                                     uint8_t y_step);

/**
 * Set the channel of interest for reading from the pixbuf. Use
 * cc3_channel_t.CC3_ALL for all channels.
 *
 * @param[in] chan The channel of interest.
 * @return \a true on success.
 */
bool cc3_pixbuf_frame_set_coi (cc3_channel_t chan);

/**
 * Reset #cc3_g_pixbuf_frame to default values.
 */
void cc3_pixbuf_frame_reset (void);

/**
 * Activate or deactivate an LED.
 *
 * \note Sometimes a LED is shared with GPIO or other functions.
 *
 * @param[in] led The LED to act on.
 * @param[in] state Set to \a true to turn the LED on, \a false
 * to turn the LED off.
 */
void cc3_led_set_state (uint8_t led, bool state);

/**
 * Set the power state of the camera and reset #cc3_g_pixbuf_frame. Used to
 * conserve power when the camera is not being used.
 *
 * \note If state is set to \a false, the camera will not work.
 *
 * @param[in] state Set to \a true for normal operation,
 * set to \a false to disable power to the camera.
 */
void cc3_camera_set_power_state (bool state);

/**
 * Set the resolution of the camera hardware and reset #cc3_g_pixbuf_frame.
 *
 * @param[in] res Resolution to set.
 */
void cc3_camera_set_resolution (cc3_camera_resolution_t res);

/**
 * Set the colorspace of the camera and reset #cc3_g_pixbuf_frame.
 *
 * @param[in] colorspace The colorspace.
 */
void cc3_camera_set_colorspace (cc3_colorspace_t colorspace);

/**
 * Set the framerate divider and reset #cc3_g_pixbuf_frame.
 *
 * @param[in] rate_divider Desired framerate divider.
 */
void cc3_camera_set_framerate_divider (uint8_t rate_divider);

/**
 * Set auto exposure and reset #cc3_g_pixbuf_frame.
 *
 * @param[in] ae Auto exposure value.
 */
void cc3_camera_set_auto_exposure (bool ae);

/**
 * Set auto white balance and reset #cc3_g_pixbuf_frame.
 *
 * @param[in] wb Auto white balance value.
 */
void cc3_camera_set_auto_white_balance (bool wb);

/**
 * Set the brightness and reset #cc3_g_pixbuf_frame.
 *
 * @param[in] level The level.
 */
void cc3_camera_set_brightness (uint8_t level);

/**
 * Set contrast and reset #cc3_g_pixbuf_frame.
 *
 * @param[in] level The level.
 */
void cc3_camera_set_contrast (uint8_t level);

/**
 * Using the camera control bus, set a camera register to a value.
 * This will take an address and a value from the OmniVision manual
 * and set it on the camera.  This should be used for advanced low level
 * manipulation of the camera modes.  Currently, this will not set the
 * corresponding cc3 internal data structure that keeps record of the camera
 * mode, nor will it change #cc3_g_pixbuf_frame.
 * \warning Use with CAUTION.
 *
 * @param[in] address The address.
 * @param[in] value The value.
 * @return \a true if successful.
 */
bool cc3_camera_set_raw_register (uint8_t address, uint8_t value);

/**
 * Initialize a serial UART. Call this for each UART to initialize.
 *
 * @param[in] uart The UART to initialize.
 * @param[in] rate The rate.
 * @param[in] mode The mode.
 * @param[in] binmode The binary/text mode.
 * @return \a true if successful.
 */
bool cc3_uart_init (uint8_t uart,
                    cc3_uart_rate_t rate,
                    cc3_uart_mode_t mode, cc3_uart_binmode_t binmode);

/**
 * Get the number of UARTs on this device.
 *
 * @return Number of UARTs.
 */
uint8_t cc3_uart_get_count (void);

/**
 * Get a file handle for a UART.
 *
 * @param[in] uart The UART to open.
 * @param[in] mode Mode as in fopen.
 * @return The file handle or \a NULL if error.
 */
FILE *cc3_uart_fopen (uint8_t uart, const char *mode);

/**
 * Do a non-blocking check to see if data is waiting on the UART.
 *
 * @param[in] uart The UART to check.
 * @return \a true if data can be read without blocking.
 */
bool cc3_uart_has_data (uint8_t uart);

/**
 * Get the value of the monotonic timer.
 *
 * @return Number of milliseconds since an arbitrary time in the past.
 */
uint32_t cc3_timer_get_current_ms (void);

/**
 * Wait for a certain amount.
 *
 * @param[in] delay Number of milliseconds to sleep.
 */
void cc3_timer_wait_ms (uint32_t delay);

/**
 * Get the value of the button right now.
 *
 * @return \a true if button is depressed.
 */
bool cc3_button_get_state (void);

/**
 * Get and reset the trigger functionality of the button.
 *
 * @return \a true if the button has been pressed since the
 * last time this function was called.
 */
bool cc3_button_get_and_reset_trigger (void);

/**
 * Set a servo to a position.
 *
 * \note If the pin is not in servo mode, this function will still
 * set the position, but will not change the mode of the pin.
 *
 * @sa cc3_gpio_set_mode() for setting a pin to servo mode.
 *
 * @param[in] pin The pin to set.
 * @param[in] position The position to set the servo.
 * @return \a true if successful.
 */
bool cc3_gpio_set_servo_position (uint8_t pin, uint8_t position);

/**
 * Get a the position of a servo.
 *
 * @param[in] pin The pin to query.
 * @return The servo position value.
 */
uint8_t cc3_gpio_get_servo_position (uint8_t pin);

/**
 * Configure a GPIO pin as input, output, or servo.
 *
 * @param[in] pin The pin to set.
 * @param[in] mode The mode to set the pin to.
 */
void cc3_gpio_set_mode (uint8_t pin, cc3_gpio_mode_t mode);

/**
 * Get the current mode setting for a GPIO pin.
 *
 * @param[in] pin The pin to assign a mode to.
 * @return The GPIO mode for this pin.
 */
cc3_gpio_mode_t cc3_gpio_get_mode(uint8_t pin);

/**
 * Set a GPIO pin to a value.
 * \note This has no effect if a pin's mode is set to input.
 * It has only a momentary effect if the pin's mode is set to servo.
 *
 * @param[in] pin The pin to set.
 * @param[in] value The value to set the pin to.
 */
void cc3_gpio_set_value(uint8_t pin, bool value);

/**
 * Read the value from a GPIO pin.
 * \note If this pin is in input mode, this will return the last value
 * the pin was set to.
 *
 * @param[in] pin The pin to get.
 * @return The value of the pin.
 */
bool cc3_gpio_get_value(uint8_t pin);

/**
 * Get the number of available GPIO pins.
 *
 * @return The number of GPIO pins.
 */
uint8_t cc3_gpio_get_count(void);


void cc3_uart0_write(const char *str);

int cc3_uart0_getchar(void);

int cc3_uart0_putchar(int c);

void cc3_uart0_write_hex(unsigned int i);

#endif
