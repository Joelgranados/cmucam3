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


/******************************************************************************
 *
 *  Initial CMUcam3 (cc3) data types and functions.
 *
 *****************************************************************************/
#include <stdbool.h>
#include <stdlib.h>

#include "cc3.h"
#include "cc3_pin_defines.h"
#include "cc3_hal.h"
#include "serial.h"
#include "devices.h"
#include "interrupt.h"


// Globals used by CMUCam functions
cc3_pixel_t cc3_g_current_pixel;        // global that gets updated with pixbuf calls
cc3_frame_t cc3_g_pixbuf_frame;        // global that keeps clip, stride


/**
 * cc3_pixbuf_rewind():
 * Rewinds the fifo.
 * Calling this and then changing parameters such as the
 * region of interest, channel of interest, virtual frame, and
 * subsampling will allow rapid reprocessing of a new frame.
 */
__attribute__((error("No FIFO")))
void cc3_pixbuf_rewind (void)
{
}


void cc3_led_set_state (uint8_t select, bool state)
{
  if (state) {
    switch (select) {
    case 0:
      REG (GPIO_IOCLR) = _CC3_LED_0;
      break;
    }
  } else {
    switch (select) {
    case 0:
      REG (GPIO_IOSET) = _CC3_LED_0;
      break;
    }
  }
}


uint8_t *cc3_malloc_rows (uint32_t rows)
{
  int channels = cc3_g_pixbuf_frame.channels;
  int width = cc3_g_pixbuf_frame.width;

  return (uint8_t *) malloc (width * channels * rows);
}


/**
 * cc3_pixbuf_read_rows():
 * Using the cc3_frame_t reads rows taking into account the virtual window and subsampling.
 * This function copies a specified number of rows from the camera FIFO into a block
 * of cc3_pixel_t memory.
 * This should be the lowest level call that the user directly interacts with.
 * Returns number of rows read. (May be zero if error.)
 */
int cc3_pixbuf_read_rows (void * mem, uint32_t rows)
{
  return 0;
}

/**
 * cc3_wait_ms():
 *
 */
void cc3_timer_wait_ms (uint32_t delay)
{
  uint32_t start;
  start = cc3_timer_get_current_ms ();
  while (cc3_timer_get_current_ms () < (start + delay));
}


uint32_t cc3_timer_get_current_ms ()
{
  return (REG (TIMER0_TC));     // REG in milliseconds
}

/**
 * cc3_pixbuf_frame_set_roi():
 * Sets the region of interest in cc3_frame_t for virtual windowing.
 * This function changes the way data is read from the FIFO.
 */
bool cc3_pixbuf_frame_set_roi (int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
  return false;
}

/**
 * cc3_pixbuf_frame_set_subsample():
 * Sets the subsampling step and mode in cc3_frame_t.
 * This function changes the way data is read from the FIFO.
 */
bool cc3_pixbuf_frame_set_subsample (cc3_subsample_mode_t mode, uint8_t x_step,
			       uint8_t y_step)
{
  return false;
}

/**
 * cc3_pixbuf_frame_set_coi():
 * Sets the channel of interest 1 or all.
 * This function changes the way data is read from the FIFO.
 */
bool cc3_pixbuf_frame_set_coi (cc3_channel_t chan)
{
  return false;
}



/**
 * cc3_camera_init():
 * First Enable Camera & FIFO Power, next Reset Camera, then call cc3_set functions for default state
 *
 */
bool cc3_camera_init ()
{
  // set pins to i2c
  REG (PCB_PINSEL0) = ((~_CC3_I2C0_MASK) & REG (PCB_PINSEL0)) | _CC3_I2C0_PINSEL;
//  REG (PCB_PINSEL0) = _CC3_I2C0_PINSEL;

  cc3_uart0_write("i2c PCB set\r\n");

 
  REG (GPIO_IOCLR) = _CC3_CAM_RESET;
  REG (GPIO_IOCLR) = _CC3_CAM_ENABLE;

  // Give everything time to settle, remove later
  cc3_timer_wait_ms(500);

  // enable EXTCLK and voltage regulators
  REG (GPIO_IOSET) = _CC3_CAM_ENABLE;

  // wait >1ms
  cc3_timer_wait_ms(1);

  // enable RESET
  REG (GPIO_IOSET) = _CC3_CAM_RESET;

  // wait >353ms for VCO to stabalize
  cc3_timer_wait_ms(353);



  return true;
}

void cc3_pixbuf_frame_reset ()
{

}



/**
 * cc3_set_raw_register():
 * This will take an address and a value from the OmniVision manual
 * and set it on the camera.  This should be used for advanced low level
 * manipulation of the camera modes.  Currently, this will not set the
 * corresponding cc3 internal data structure that keeps record of the camera
 * mode.  Use with CAUTION.
 *
 * For basic manipulation of camera parameters see other cc3_set_xxxx functions.
 */
bool cc3_camera_set_raw_register (uint8_t address, uint8_t value)
{
  return false;
}


/**
 * Sets the resolution, cc3_g_pixbuf_frame width and height not updated
 * until next pixbuf load.
 * Takes enum CC3_RES_LOW and CC3_RES_HIGH.
 */
void cc3_camera_set_resolution (cc3_camera_resolution_t cam_res)
{
  
}

void cc3_camera_set_power_state (bool state)
{
  
}

void cc3_camera_set_colorspace (cc3_colorspace_t colorspace)
{
  
}


void cc3_camera_set_framerate_divider (uint8_t rate_divider)
{
  
}

void cc3_camera_set_auto_exposure (bool exp)
{
  
}

void cc3_camera_set_auto_white_balance (bool awb)
{
  
}

void cc3_camera_set_brightness (uint8_t level)
{
  
}

void cc3_camera_set_contrast (uint8_t level)
{
  
}


bool cc3_button_get_state (void)
{
  if (!_cc3_button_trigger) {
    // button has not been pressed
    return false;
  }

  // otherwise, it has been pressed, and it's in GPIO mode
  return !(REG (GPIO_IOPIN) & _CC3_BUTTON);
}

bool cc3_button_get_and_reset_trigger (void)
{
  bool result = _cc3_button_trigger;
  _cc3_button_trigger = false;

  // reset interrupt
  if (result) {
    enable_button_interrupt();
  }

  return result;
}
