/*
 * Copyright 2006  Anthony Rowe and Adam Goode
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


static inline void _cc3_seek_left (void);
static inline void _cc3_seek_top (void);

static inline void _cc3_pixbuf_skip_pixels (uint32_t size);

static inline uint8_t _cc3_pixbuf_read_subpixel (void);

static inline void _cc3_pixbuf_read_pixel (uint8_t * pixel,
                                           uint8_t * saved,
                                           uint8_t off0,
                                           uint8_t off1, uint8_t off2);

static inline void _cc3_pixbuf_skip_subpixel (void);


// Move to the next byte in the FIFO
static inline void _cc3_fifo_read_inc (void);

static uint8_t _cc3_second_green;
static bool _cc3_second_green_valid;

static void _cc3_update_frame_bounds (cc3_frame_t *);

static void _cc3_pixbuf_resize (void);

void cc3_pixbuf_load ()
{
  //  uint32_t start_time;

  unsigned int i;

  if (cc3_g_pixbuf_frame.reset_on_next_load) {
    _cc3_pixbuf_resize();
  }

  //REG(GPIO_IOCLR)=CAM_IE;
  //while(frame_done!=1);
 // cc3_pixbuf_rewind();
  _cc3_pixbuf_write_rewind ();

  //  start_time = cc3_timer();
  while (!(REG (GPIO_IOPIN) & _CC3_CAM_VSYNC)); //while(CAM_VSYNC);
  while (REG (GPIO_IOPIN) & _CC3_CAM_VSYNC);    //while(!CAM_VSYNC);


  REG (GPIO_IOSET) = _CC3_BUF_WEE;
  //  printf("vsync wait: %3d ms\r", cc3_timer() - start_time);


  // wait for vsync to finish
  while (!(REG (GPIO_IOPIN) & _CC3_CAM_VSYNC)); //while(CAM_VSYNC);

  enable_ext_interrupt ();

  // Delay to wait for the image buffer to fill up before you start reading it
  for (i = 0; i < 3; i++) {
    while (!(REG (GPIO_IOPIN) & _CC3_CAM_HREF));
    while (REG (GPIO_IOPIN) & _CC3_CAM_HREF);
  }

  cc3_pixbuf_rewind();
  cc3_g_pixbuf_frame.y_loc = 0;

  //while (REG (GPIO_IOPIN) & _CC3_CAM_VSYNC);
  //REG (GPIO_IOCLR) = _CC3_BUF_WEE;
//      delay();
//  REG(GPIO_IOCLR)=_CC3_BUF_WEE;  //BUF_WEE=0return 1;
}

void _cc3_fifo_read_inc (void)
{
  REG (GPIO_IOSET) = _CC3_BUF_RCK;
  REG (GPIO_IOCLR) = _CC3_BUF_RCK;
}

void _cc3_pixbuf_skip_pixels (uint32_t size)
{
  uint32_t i;

  for (i = 0; i < size; i++) {
    _cc3_pixbuf_skip_subpixel ();
    _cc3_pixbuf_skip_subpixel ();
    _cc3_pixbuf_skip_subpixel ();
    _cc3_pixbuf_skip_subpixel ();
  }
}


void _cc3_seek_top ()
{
  if (cc3_g_pixbuf_frame.y_loc < cc3_g_pixbuf_frame.y0) {
    _cc3_pixbuf_skip_pixels (cc3_g_pixbuf_frame.raw_width / 2
			     * cc3_g_pixbuf_frame.y0);

    cc3_g_pixbuf_frame.y_loc = cc3_g_pixbuf_frame.y0;
  }
}


void _cc3_seek_left ()
{
  _cc3_pixbuf_skip_pixels (cc3_g_pixbuf_frame.x0 / 2);
}

uint8_t _cc3_pixbuf_read_subpixel (void)
{
  uint8_t result = REG (GPIO_IOPIN) >> 24;
  _cc3_fifo_read_inc ();

  return result;
}

void _cc3_pixbuf_skip_subpixel (void)
{
  _cc3_fifo_read_inc ();
}

void _cc3_pixbuf_read_pixel (uint8_t * pixel,
                             uint8_t * saved,
                             uint8_t off0, uint8_t off1, uint8_t off2)
{
  if (cc3_g_pixbuf_frame.x_step == 1) {
    if (_cc3_second_green_valid) {
      // use the second green
      _cc3_second_green_valid = false;
      *(pixel + off0) = *(saved + off0);
      *(pixel + off1) = _cc3_second_green;
      *(pixel + off2) = *(saved + off2);

      return;
    }

    // otherwise, load a new thing
    *(pixel + off1) = _cc3_pixbuf_read_subpixel ();   // G
    *(pixel + off0) = _cc3_pixbuf_read_subpixel ();   // R
    _cc3_second_green = _cc3_pixbuf_read_subpixel (); // G
    *(pixel + off2) = _cc3_pixbuf_read_subpixel ();   // B

    _cc3_second_green_valid = true;
  } else {
    _cc3_pixbuf_skip_subpixel ();
    *(pixel + off0) = _cc3_pixbuf_read_subpixel ();
    *(pixel + off1) = _cc3_pixbuf_read_subpixel ();
    *(pixel + off2) = _cc3_pixbuf_read_subpixel ();
  }
}

void _cc3_pixbuf_resize ()
{
  cc3_g_pixbuf_frame.raw_width = _cc3_g_current_camera_state.raw_width;
  cc3_g_pixbuf_frame.raw_height = _cc3_g_current_camera_state.raw_height;
  cc3_pixbuf_frame_reset();
}

/**
 * cc3_pixbuf_rewind():
 * Rewinds the fifo.
 * Calling this and then changing parameters such as the
 * region of interest, channel of interest, virtual frame, and
 * subsampling will allow rapid reprocessing of a new frame.
 */
void cc3_pixbuf_rewind ()
{
  REG (GPIO_IOCLR) = _CC3_BUF_RRST;
  REG (GPIO_IOCLR) = _CC3_BUF_RCK;
  REG (GPIO_IOSET) = _CC3_BUF_RCK;
  REG (GPIO_IOCLR) = _CC3_BUF_RCK;
  REG (GPIO_IOSET) = _CC3_BUF_RRST;
  // skip first line because it is blue
  _cc3_pixbuf_skip_pixels (cc3_g_pixbuf_frame.raw_width / 2);
  //_cc3_pixbuf_skip_pixels (cc3_g_pixbuf_frame.raw_width / 2);
  _cc3_second_green_valid = false;
  cc3_g_pixbuf_frame.y_loc = 0;
}


void cc3_clr_led (uint8_t select)
{
  switch (select) {
  case 0:
    REG (GPIO_IOCLR) = _CC3_LED_0;
    break;
  case 1:
    REG (GPIO_IOCLR) = _CC3_LED_1;
    break;
  case 2:
    REG (GPIO_IOCLR) = _CC3_LED_2;
    break;
  }

}


void cc3_set_led (uint8_t select)
{

  switch (select) {
  case 0:
    REG (GPIO_IOSET) = _CC3_LED_0;
    break;
  case 1:
    REG (GPIO_IOSET) = _CC3_LED_1;
    break;
  case 2:
    REG (GPIO_IOSET) = _CC3_LED_2;
    break;
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

  int16_t j;
  uint16_t r;

  uint8_t off0, off1, off2;

  int width = cc3_g_pixbuf_frame.width;

  unsigned int row_limit = (cc3_g_pixbuf_frame.y1 - cc3_g_pixbuf_frame.y_loc)
    / cc3_g_pixbuf_frame.y_step;

  if (row_limit < rows) {
    rows = row_limit;
  }

  if (_cc3_g_current_camera_state.colorspace == CC3_COLORSPACE_RGB) {
    off0 = 0;
    off1 = 1;
    off2 = 2;
  }
  else if (_cc3_g_current_camera_state.colorspace == CC3_COLORSPACE_YCRCB) {
    off0 = 1;
    off1 = 0;
    off2 = 2;
  }
  else {
    off0 = 0;
    off1 = 1;
    off2 = 2;
  }

  // First read into frame
  _cc3_seek_top ();

  for (r = 0; r < rows; r++) {
    int x = cc3_g_pixbuf_frame.x0;

    // First read into line
    _cc3_seek_left ();

    switch (cc3_g_pixbuf_frame.coi) {
    case CC3_CHANNEL_ALL:
      _cc3_second_green_valid = false;
      for (j = 0; j < width; j++) {
        uint8_t *p = ((uint8_t *) mem) +
          (r * width + j * 3);
        _cc3_pixbuf_read_pixel (p, p - 3, off0, off1, off2);

	// advance by x_step
	x += cc3_g_pixbuf_frame.x_step;
	_cc3_pixbuf_skip_pixels ((cc3_g_pixbuf_frame.x_step - 1) / 2);
      }

      break;

    case CC3_CHANNEL_RED:
      for (j = 0; j < width; j++) {
	uint8_t *p = ((uint8_t *) mem) + (r * width + j);

	if ((j & 0x1) == 0 || cc3_g_pixbuf_frame.x_step > 1) {
	  // read
	  _cc3_pixbuf_skip_subpixel ();
	  *p = _cc3_pixbuf_read_subpixel ();
	  _cc3_pixbuf_skip_subpixel ();
	  _cc3_pixbuf_skip_subpixel ();
	} else {
	  *p = *(p - 1);
	}

	x += cc3_g_pixbuf_frame.x_step;
	_cc3_pixbuf_skip_pixels ((cc3_g_pixbuf_frame.x_step - 1) / 2);
      }
      break;

    case CC3_CHANNEL_GREEN:
      for (j = 0; j < width; j++) {
	uint8_t *p = ((uint8_t *) mem) + (r * width + j);

	if ((j & 0x1) == 0 || cc3_g_pixbuf_frame.x_step > 1) {
	  // read
	  *p = _cc3_pixbuf_read_subpixel ();
	  _cc3_pixbuf_skip_subpixel ();
	  _cc3_second_green = _cc3_pixbuf_read_subpixel ();
	  _cc3_pixbuf_skip_subpixel ();
	} else {
	  *p = _cc3_second_green;
	}

	x += cc3_g_pixbuf_frame.x_step;
	_cc3_pixbuf_skip_pixels ((cc3_g_pixbuf_frame.x_step - 1) / 2);
      }
      break;

    case CC3_CHANNEL_BLUE:
      for (j = 0; j < width; j++) {
	uint8_t *p = ((uint8_t *) mem) + (r * width + j);

	if ((j & 0x1) == 0 || cc3_g_pixbuf_frame.x_step > 1) {
	  // read
	  _cc3_pixbuf_skip_subpixel ();
	  _cc3_pixbuf_skip_subpixel ();
	  _cc3_pixbuf_skip_subpixel ();
	  *p = _cc3_pixbuf_read_subpixel ();
	} else {
	  *p = *(p - 1);
	}

	x += cc3_g_pixbuf_frame.x_step;
	_cc3_pixbuf_skip_pixels ((cc3_g_pixbuf_frame.x_step - 1) / 2);
      }
      break;
    }
    _cc3_pixbuf_skip_pixels ((cc3_g_pixbuf_frame.raw_width - x) / 2);


    // advance by y_step
    _cc3_pixbuf_skip_pixels ((cc3_g_pixbuf_frame.y_step - 1) * cc3_g_pixbuf_frame.raw_width / 2);
    cc3_g_pixbuf_frame.y_loc += cc3_g_pixbuf_frame.y_step;
  }
  return rows;
}

/**
 * cc3_wait_ms():
 *
 */
void cc3_wait_ms (uint32_t delay)
{
  uint32_t start;
  start = cc3_timer ();
  while (cc3_timer () < (start + delay));
}


/**
 * cc3_timer():
 *
 * This function returns the time since startup in ms as a uint32
 */
uint32_t cc3_timer ()
{
  return (REG (TIMER0_TC));     // REG in milliseconds
}

/**
 * cc3_pixbuf_set_roi():
 * Sets the region of interest in cc3_frame_t for virtual windowing.
 * This function changes the way data is read from the FIFO.
 */
bool cc3_pixbuf_set_roi (int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
  int w = cc3_g_pixbuf_frame.raw_width;
  int h = cc3_g_pixbuf_frame.raw_height;

  // constrain
  if (x0 < 0) {
    x0 = 0;
  }
  if ((x0 & 0x1) == 1) {
    x0++;          // x0 must be even!
  }
  if (y0 < 0) {
    y0 = 0;
  }
  if (x0 > w) {
    x0 = w;
  }
  if (y0 > h) {
    y0 = h;
  }

  if (x1 < 0) {
    x1 = 0;
  }
  if (y1 < 0) {
    y1 = 0;
  }
  if (x1 > w) {
    x1 = w;
  }
  if (y1 > h) {
    y1 = h;
  }

  // check bounds
  if (x0 >= x1 || y0 >= y1) {
    return false;
  }

  // set if ok
  cc3_g_pixbuf_frame.x0 = x0;
  cc3_g_pixbuf_frame.y0 = y0;
  cc3_g_pixbuf_frame.x1 = x1;
  cc3_g_pixbuf_frame.y1 = y1;

  _cc3_update_frame_bounds (&cc3_g_pixbuf_frame);

  return true;
}

/**
 * cc3_pixbuf_set_subsample():
 * Sets the subsampling step and mode in cc3_frame_t.
 * This function changes the way data is read from the FIFO.
 */
bool cc3_pixbuf_set_subsample (cc3_subsample_mode_t mode, uint8_t x_step,
			       uint8_t y_step)
{
  bool result = true;

  if (x_step == 0) {
    x_step = 1;
    result = false;
  }
  if (y_step == 0) {
    y_step = 1;
    result = false;
  }

  // only allow even subsampling (or 1) for x
  if (x_step != 1 && x_step % 2 != 0) {
    x_step++;
    result = false;
  }

  cc3_g_pixbuf_frame.x_step = x_step;
  cc3_g_pixbuf_frame.y_step = y_step;
  //cc3_g_pixbuf_frame.x0 = 0;
  //cc3_g_pixbuf_frame.y0 = 0;
  //cc3_g_pixbuf_frame.x1 = cc3_g_pixbuf_frame.raw_width;
  //cc3_g_pixbuf_frame.y1 = cc3_g_pixbuf_frame.raw_height;
  cc3_g_pixbuf_frame.subsample_mode = mode;

  _cc3_update_frame_bounds (&cc3_g_pixbuf_frame);

  return result;
}

/**
 * cc3_pixbuf_set_coi():
 * Sets the channel of interest 1 or all.
 * This function changes the way data is read from the FIFO.
 */
bool cc3_pixbuf_set_coi (cc3_channel_t chan)
{
  if (chan > 4)
    return false;                   // Sanity check on bounds
  cc3_g_pixbuf_frame.coi = chan;

  cc3_g_pixbuf_frame.channels = (chan == CC3_CHANNEL_ALL ? 3 : 1);

  return true;
}



/**
 * cc3_camera_init():
 * First Enable Camera & FIFO Power, next Reset Camera, then call cc3_set functions for default state
 *
 */
bool cc3_camera_init ()
{
  REG (PCB_PINSEL0) = (REG (PCB_PINSEL0) & 0xFFFF0000) | UART0_PCB_PINSEL_CFG | UART1_PCB_PINSEL_CFG;  //| 0x50;
  //REG (PCB_PINSEL0) = (REG (PCB_PINSEL0) & 0xFFFF0000) | UART0_PCB_PINSEL_CFG | UART1_PCB_PINSEL_CFG; //| 0x50;
  REG (GPIO_IODIR) = _CC3_DEFAULT_PORT_DIR;
  //REG(GPIO_IOSET)=CAM_BUF_ENABLE;
  //REG (GPIO_IOCLR) = CAM_BUF_ENABLE;  // Change for AL440B
  REG (GPIO_IOCLR) = _CC3_BUF_RESET;
  _cc3_camera_reset ();
  _cc3_fifo_reset ();

  _cc3_g_current_camera_state.camera_type = _CC3_OV6620;        // XXX add autodetect code
  _cc3_g_current_camera_state.clock_divider = 0;
  _cc3_g_current_camera_state.brightness = -1;
  _cc3_g_current_camera_state.contrast = -1;
  _cc3_g_current_camera_state.auto_exposure = true;
  _cc3_g_current_camera_state.auto_white_balance = false;
  _cc3_g_current_camera_state.colorspace = CC3_COLORSPACE_RGB;
  _cc3_set_register_state ();

  _cc3_pixbuf_resize();

  return true;
}

void cc3_pixbuf_frame_reset ()
{
  cc3_g_pixbuf_frame.x_step = 1;
  cc3_g_pixbuf_frame.y_step = 1;
  cc3_g_pixbuf_frame.x0 = 0;
  cc3_g_pixbuf_frame.y0 = 0;
  cc3_g_pixbuf_frame.x1 = cc3_g_pixbuf_frame.raw_width;
  cc3_g_pixbuf_frame.y1 = cc3_g_pixbuf_frame.raw_height;
  cc3_g_pixbuf_frame.y_loc = 0;
  cc3_g_pixbuf_frame.subsample_mode = CC3_SUBSAMPLE_NEAREST;
  cc3_g_pixbuf_frame.reset_on_next_load = false;

  cc3_pixbuf_set_coi(CC3_CHANNEL_ALL);

  _cc3_update_frame_bounds (&cc3_g_pixbuf_frame);
}

/**
 * cc3_camera_kill():
 * Turn camera power off
 * Turn fifo power off (may "cause picture to evaporate")
 */
void cc3_camera_kill ()
{
// XXX I need to be implemented

}


void
cc3_system_setup (void)
{

  // --- enable and connect the PLL (Phase Locked Loop) ---
  // a. set multiplier and divider
  REG(SYSCON_PLLCFG) = MSEL | (1<<PSEL1) | (0<<PSEL0);
  // b. enable PLL
  REG(SYSCON_PLLCON) = (1<<PLLE);
  // c. feed sequence
  REG(SYSCON_PLLFEED) = PLL_FEED1;
  REG(SYSCON_PLLFEED) = PLL_FEED2;
  // d. wait for PLL lock (PLOCK bit is set if locked)
  while (!(REG(SYSCON_PLLSTAT) & (1<<PLOCK)));
  // e. connect (and enable) PLL
  REG(SYSCON_PLLCON) = (1<<PLLE) | (1<<PLLC);
  // f. feed sequence
  REG(SYSCON_PLLFEED) = PLL_FEED1;
  REG(SYSCON_PLLFEED) = PLL_FEED2;

  // --- setup and enable the MAM (Memory Accelerator Module) ---
  // a. start change by turning of the MAM (redundant)
  REG(MAMCR) = 0;
  // b. set MAM-Fetch cycle to 3 cclk as recommended for >40MHz
  REG(MAMTIM) = MAM_FETCH;
  // c. enable MAM
  REG(MAMCR) = MAM_MODE;

  // --- set VPB speed ---
  REG(SYSCON_VPBDIV) = VPBDIV_VAL;

  // --- map INT-vector ---
  REG(SYSCON_MEMMAP) = MEMMAP_USER_FLASH_MODE;

  //REG(PCB_PINSEL1) = 0x1;  // External interrupt 0

  // Setup timer0 to count by milliseconds starting from 0
  REG(TIMER0_TCR)=0;   // turn off timer
  REG(TIMER0_MCR)=0;    // disable interrupt
  REG(TIMER0_TC)=0;    // clear counter
  REG(TIMER0_PC)=0;    // clear prescale count
  REG(TIMER0_PR) = (int)((FOSC*PLL_M)/1000);  // every 1 ms
  REG(TIMER0_TCR)=1;   // start the timer
}


static void _cc3_set_cam_ddr_sccb_idle (void)
{
  REG (GPIO_IODIR) = _CC3_SCCB_PORT_DDR_IDLE;
  _cc3_delay_sccb ();
}

static void _cc3_set_cam_ddr_sccb_write (void)
{
  REG (GPIO_IODIR) = _CC3_SCCB_PORT_DDR_WRITE;
  _cc3_delay_sccb ();
}


static void _cc3_set_cam_ddr (volatile unsigned long val)
{
  //DDR(SCCB_PORT,val);
  REG (GPIO_IODIR) = val;
  _cc3_delay_sccb ();
}



static unsigned int _cc3_sccb_send (unsigned int num, unsigned int *buffer)
{
  unsigned int ack, i, k;
  unsigned int data;

  // Send Start Bit
  //SCCB_SDA=0;  // needed because values can be reset by read-modify cycle
  REG (GPIO_IOCLR) = 0x00800000;
  _cc3_set_cam_ddr (_CC3_SCCB_PORT_DDR_READ_SCL);        // SDA=0 SCL=1
  //SCCB_SCL=0;  // needed because values can be reset by read-modify cycle
  REG (GPIO_IOCLR) = 0x00400000;
  _cc3_set_cam_ddr_sccb_write ();        // SDA=0 SCL=0

  // Send the Byte
  for (k = 0; k != num; k++) {
    data = buffer[k];           // To avoid shifting array problems
    for (i = 0; !(i & 8); i++)  // Write data
    {
      if (data & 0x80) {
        _cc3_set_cam_ddr (_CC3_SCCB_PORT_DDR_READ_SDA);  // SDA=1 SCL=0
        _cc3_set_cam_ddr_sccb_idle ();   // SDA=1 SCL=1
      }
      else {
        _cc3_set_cam_ddr_sccb_write ();  // SDA=0 SCL=0
        _cc3_set_cam_ddr (_CC3_SCCB_PORT_DDR_READ_SCL);  // SDA=0 SCL=1

      }
      while (!(REG (GPIO_IOPIN) & 0x00400000));
      //while(!SCCB_SCL);


      if (data & 0x08) {
        _cc3_set_cam_ddr (_CC3_SCCB_PORT_DDR_READ_SDA);  // SDA=1 SCL=0

      }
      else {
        _cc3_set_cam_ddr_sccb_write ();  // SDA=0 SCL=0
      }

      data <<= 1;
    }                           // END OF 8 BIT FOR LOOP

    // Check ACK  <*************************************
    _cc3_set_cam_ddr (_CC3_SCCB_PORT_DDR_READ_SDA);      // SDA=1 SCL=0

    _cc3_set_cam_ddr_sccb_idle ();       // SDA=1 SCL=1
    ack = 0;

    //if(SCCB_SDA)                     // sample SDA
    if (REG (GPIO_IOPIN) & 0x00800000) {
      ack |= 1;
      break;
    }

    _cc3_set_cam_ddr_sccb_write ();      // SDA=0 SCL=0

  }

  // Send Stop Bit
  _cc3_set_cam_ddr (_CC3_SCCB_PORT_DDR_READ_SCL);        // SDA=0 SCL=1
  _cc3_set_cam_ddr_sccb_idle (); // SDA=1 SCL=1

  return ack;

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
bool cc3_set_raw_register (uint8_t address, uint8_t value)
{
  unsigned int data[3];
  int to;
  data[0] = _cc3_g_current_camera_state.camera_type;
  data[1] = address;
  data[2] = value;
  to = 0;
  while (_cc3_sccb_send (3, data)) {
    to++;
    if (to > 3)
      return false;
  }
  _cc3_delay_us_4 (1);
  return true;
}


/**
 * Sets the resolution, cc3_g_pixbuf_frame width and height not updated
 * until next pixbuf load.
 * Takes enum CC3_RES_LOW and CC3_RES_HIGH.
 */
void cc3_set_resolution (cc3_camera_resolution_t cam_res)
{
  _cc3_g_current_camera_state.resolution = cam_res;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
  cc3_g_pixbuf_frame.reset_on_next_load = true;
}

void _cc3_update_frame_bounds (cc3_frame_t *f)
{
  f->width = (f->x1 - f->x0) / f->x_step;
  f->height = (f->y1 - f->y0) / f->y_step;
}


void cc3_set_colorspace (cc3_colorspace_t colorspace)
{
  _cc3_g_current_camera_state.colorspace = colorspace;
  _cc3_set_register_state ();
}


void cc3_set_framerate_divider (uint8_t rate_divider)
{
  _cc3_g_current_camera_state.clock_divider = rate_divider;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
}

void cc3_set_auto_exposure (bool exp)
{
  _cc3_g_current_camera_state.auto_exposure = exp;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
}

void cc3_set_auto_white_balance (bool awb)
{
  _cc3_g_current_camera_state.auto_white_balance = awb;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
}

void cc3_set_brightness (uint8_t level)
{
  _cc3_g_current_camera_state.brightness = level;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
  return 1;
}

void cc3_set_contrast (uint8_t level)
{
  _cc3_g_current_camera_state.contrast = level;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
  return 1;
}


bool cc3_read_button (void)
{
  bool result = !(REG (GPIO_IOPIN) & _CC3_BUTTON);
  return result;
}
