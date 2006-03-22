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
cc3_frame_t cc3_g_current_frame;        // global that keeps clip, stride


static inline void _cc3_seek_left (void);
static inline void _cc3_seek_right_down (void);
static inline void _cc3_seek_top (void);
static inline void _cc3_advance_x_loc (void);

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

void cc3_pixbuf_load ()
{
  unsigned int i;
  //REG(GPIO_IOCLR)=CAM_IE;  
  //while(frame_done!=1);
  cc3_pixbuf_rewind ();
  _cc3_pixbuf_write_rewind ();
  while (!(REG (GPIO_IOPIN) & _CC3_CAM_VSYNC)); //while(CAM_VSYNC);
  while (REG (GPIO_IOPIN) & _CC3_CAM_VSYNC);    //while(!CAM_VSYNC);

  REG (GPIO_IOSET) = _CC3_BUF_WEE;

  // wait for vsync to finish
  while (!(REG (GPIO_IOPIN) & _CC3_CAM_VSYNC)); //while(CAM_VSYNC);

  enable_ext_interrupt ();
  for (i = 0; i < 3; i++) {
    while (!(REG (GPIO_IOPIN) & _CC3_CAM_HREF));
    while (REG (GPIO_IOPIN) & _CC3_CAM_HREF);
  }
  cc3_g_current_frame.x_loc = 0;
  cc3_g_current_frame.y_loc = 0;
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
  for (i = 0; i < size * 4; i++) {
    _cc3_fifo_read_inc ();
  }
}

/**
 * cc3_pixbuf_read():
 * loads cc3_g_current_pixel from fifo
 * Returns 1 upon success and 0 upon a bounds failure
 * This is slow, but takes care of channel of interest, downsampling, and virtual bounding boxes
 */

int cc3_pixbuf_read ()
{
  return 0;
}

void _cc3_seek_top ()
{
  int8_t i;

  // Skip top
  for (cc3_g_current_frame.y_loc = 0;
       cc3_g_current_frame.y_loc < cc3_g_current_frame.y0;
       cc3_g_current_frame.y_loc++)
    for (i = 0; i < cc3_g_current_frame.y_step; i++)
      _cc3_pixbuf_skip_pixels (cc3_g_current_frame.raw_width);
  cc3_g_current_frame.x_loc = 0;
}


void _cc3_seek_left ()
{
  // Skip left 
  if (cc3_g_current_frame.x_loc < cc3_g_current_frame.x0) {
    _cc3_pixbuf_skip_pixels (cc3_g_current_frame.x0
                             * cc3_g_current_frame.x_step);
    cc3_g_current_frame.x_loc = cc3_g_current_frame.x0;
  }
}

void _cc3_seek_right_down ()
{
  int16_t j;
  if (cc3_g_current_frame.x_loc < cc3_g_current_frame.x1) {
    // no need for seeking
    return;
  }

  // Skip right and down 
  _cc3_pixbuf_skip_pixels (cc3_g_current_frame.raw_width -
                           (cc3_g_current_frame.x_loc *
                            cc3_g_current_frame.x_step));
  cc3_g_current_frame.x_loc = 0;
  cc3_g_current_frame.y_loc++;
  if (cc3_g_current_frame.y_step > 1) {
    // skip horizontally down the image
    for (j = 0; j < cc3_g_current_frame.y_step - 1; j++)
      _cc3_pixbuf_skip_pixels (cc3_g_current_frame.raw_width);
  }
}

void _cc3_advance_x_loc ()
{
  cc3_g_current_frame.x_loc++;
  // skip horizontally for next read
  if (cc3_g_current_frame.x_step > 1) {
    if (cc3_g_current_frame.raw_width >
        (cc3_g_current_frame.x_loc + cc3_g_current_frame.x_step)) {
      // skip pixel
      _cc3_pixbuf_skip_pixels (cc3_g_current_frame.x_step - 1);
    }
  }
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
  if (_cc3_second_green_valid) {
    // use the second green
    _cc3_second_green_valid = false;
    *(pixel + off0) = *(saved + off0);
    *(pixel + off1) = _cc3_second_green;
    *(pixel + off2) = *(saved + off2);

    return;
  }

  // otherwise, load a new thing
  _cc3_second_green = _cc3_pixbuf_read_subpixel ();
  *(pixel + off0) = _cc3_pixbuf_read_subpixel ();
  *(pixel + off1) = _cc3_pixbuf_read_subpixel ();
  *(pixel + off2) = _cc3_pixbuf_read_subpixel ();

  _cc3_second_green_valid = true;
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

  _cc3_second_green_valid = false;
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


uint8_t *cc3_malloc_row (void)
{
  int channels;
  if (cc3_g_current_frame.coi == CC3_ALL) {
    channels = 3;
  }
  else {
    channels = 1;
  }

  return (uint8_t *) malloc (cc3_g_current_frame.width * channels);
}


/**
 * cc3_pixbuf_read_rows():
 * Using the cc3_frame_t reads rows taking into account the virtual window and subsampling. 
 * This function copies a specified number of rows from the camera FIFO into a block
 * of cc3_pixel_t memory.
 * This should be the lowest level call that the user directly interacts with.
 * Returns 1 upon success
 * Returns -1 if requesting too many rows
 */
int cc3_pixbuf_read_rows (void *mem, uint32_t rows)
{

  int16_t j;
  uint16_t r;

  uint8_t off0, off1, off2;

  if ((cc3_g_current_frame.y0 + rows) > cc3_g_current_frame.y1)
    return -1;

  if (_cc3_g_current_camera_state.colorspace == CC3_RGB) {
    off0 = 0;
    off1 = 1;
    off2 = 2;
  }
  else if (_cc3_g_current_camera_state.colorspace == CC3_YCRCB) {
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
    // First read after line
    _cc3_seek_right_down ();

    // First read into line
    _cc3_seek_left ();

    if (cc3_g_current_frame.coi == CC3_ALL) {
      for (j = 0; j < cc3_g_current_frame.width; j++) {
        uint8_t *p = ((uint8_t *) mem) +
          (r * cc3_g_current_frame.width + j * 3);
        _cc3_pixbuf_read_pixel (p, p - 3, off0, off1, off2);
        _cc3_advance_x_loc ();
      }
    }
    else {
      // ...
    }
  }
  return 1;
}

/**
 * cc3_wait_ms():
 *
 * This function returns the time since startup in ms as a uint32
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
 * Returns 1 upon success and 0 on an out of bounds failure.
 */
int cc3_pixbuf_set_roi (int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
  if (x0 >= 0
      && x0 <= (cc3_g_current_frame.raw_width / cc3_g_current_frame.x_step)
      && y0 >= 0
      && y0 <= (cc3_g_current_frame.raw_height / cc3_g_current_frame.y_step)
      && x1 >= 0
      && x1 <= (cc3_g_current_frame.raw_width / cc3_g_current_frame.x_step)
      && y1 >= 0
      && y1 <= (cc3_g_current_frame.raw_height / cc3_g_current_frame.y_step)
      && x0 < x1 && y0 < y1) {
    cc3_g_current_frame.x0 = x0;
    cc3_g_current_frame.y0 = y0;
    cc3_g_current_frame.x1 = x1;
    cc3_g_current_frame.y1 = y1;
    cc3_g_current_frame.width = x1 - x0;
    cc3_g_current_frame.height = y1 - y0;
    return 1;
  }
  return 0;
}

/**
 * cc3_pixbuf_set_subsample():
 * Sets the subsampling step and mode in cc3_frame_t. 
 * This function changes the way data is read from the FIFO.
 * Warning:  This function resets the ROI to the size of the new image.
 */
int cc3_pixbuf_set_subsample (cc3_subsample_mode_t mode, uint8_t x_step,
                              uint8_t y_step)
{
  // can't be < 0 if unsigned
  /* 
     if (x_step < 0 || y_step < 0)
     return 0;
   */
  cc3_g_current_frame.x_step = x_step;
  cc3_g_current_frame.y_step = y_step;
  cc3_g_current_frame.width = cc3_g_current_frame.raw_width / x_step;
  cc3_g_current_frame.height = cc3_g_current_frame.raw_height / y_step;
  cc3_g_current_frame.x0 = 0;
  cc3_g_current_frame.y0 = 0;
  cc3_g_current_frame.x1 = cc3_g_current_frame.width;
  cc3_g_current_frame.y1 = cc3_g_current_frame.height;
  cc3_g_current_frame.subsample_mode = mode;
  return 1;
}

/**
 * cc3_pixbuf_set_coi():
 * Sets the channel of interest 1 or all.
 * This function changes the way data is read from the FIFO.
 * Returns 1 upon success and 0 on failure.
 */
int cc3_pixbuf_set_coi (cc3_channel_t chan)
{
  if (chan > 4)
    return 0;                   // Sanity check on bounds
  cc3_g_current_frame.coi = chan;
  return 1;
}



/**
 * cc3_camera_init():
 * First Enable Camera & FIFO Power, next Reset Camera, then call cc3_set functions for default state 
 *
 * Return:
 * 	1 successfully got acks back
 * 	0 failure (probably due to hardware?)
 */
int cc3_camera_init ()
{
  REG (PCB_PINSEL0) = (REG (PCB_PINSEL0) & 0xFFFF0000) | UART0_PCB_PINSEL_CFG;  //| 0x50;
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
  _cc3_g_current_camera_state.colorspace = CC3_RGB;
  _cc3_set_register_state ();

  cc3_frame_default ();

  return 1;
}

void cc3_frame_default ()
{
  cc3_g_current_frame.coi = CC3_ALL;
  cc3_g_current_frame.x_step = 1;
  cc3_g_current_frame.y_step = 1;
  cc3_g_current_frame.x0 = 0;
  cc3_g_current_frame.y0 = 0;
  cc3_g_current_frame.width = cc3_g_current_frame.raw_width;
  cc3_g_current_frame.height = cc3_g_current_frame.raw_height;
  cc3_g_current_frame.x1 = cc3_g_current_frame.raw_width;
  cc3_g_current_frame.y1 = cc3_g_current_frame.raw_height;
  cc3_g_current_frame.x_loc = 0;
  cc3_g_current_frame.y_loc = 0;
  cc3_g_current_frame.subsample_mode = CC3_NEAREST;
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


static void _cc3_set_cam_ddr_i2c_idle (void)
{
  REG (GPIO_IODIR) = _CC3_I2C_PORT_DDR_IDLE;
  _cc3_delay_i2c ();
}

static void _cc3_set_cam_ddr_i2c_write (void)
{
  REG (GPIO_IODIR) = _CC3_I2C_PORT_DDR_WRITE;
  _cc3_delay_i2c ();
}


static void _cc3_set_cam_ddr (volatile unsigned long val)
{
  //DDR(I2C_PORT,val);
  REG (GPIO_IODIR) = val;
  _cc3_delay_i2c ();
}



static unsigned int _cc3_i2c_send (unsigned int num, unsigned int *buffer)
{
  unsigned int ack, i, k;
  unsigned int data;

  // Send Start Bit
  //I2C_SDA=0;  // needed because values can be reset by read-modify cycle
  REG (GPIO_IOCLR) = 0x00800000;
  _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SCL);        // SDA=0 SCL=1
  //I2C_SCL=0;  // needed because values can be reset by read-modify cycle
  REG (GPIO_IOCLR) = 0x00400000;
  _cc3_set_cam_ddr_i2c_write ();        // SDA=0 SCL=0

  // Send the Byte    
  for (k = 0; k != num; k++) {
    data = buffer[k];           // To avoid shifting array problems   
    for (i = 0; !(i & 8); i++)  // Write data    
    {
      if (data & 0x80) {
        _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SDA);  // SDA=1 SCL=0
        _cc3_set_cam_ddr_i2c_idle ();   // SDA=1 SCL=1
      }
      else {
        _cc3_set_cam_ddr_i2c_write ();  // SDA=0 SCL=0
        _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SCL);  // SDA=0 SCL=1

      }
      while (!(REG (GPIO_IOPIN) & 0x00400000));
      //while(!I2C_SCL);


      if (data & 0x08) {
        _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SDA);  // SDA=1 SCL=0

      }
      else {
        _cc3_set_cam_ddr_i2c_write ();  // SDA=0 SCL=0
      }

      data <<= 1;
    }                           // END OF 8 BIT FOR LOOP

    // Check ACK  <*************************************
    _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SDA);      // SDA=1 SCL=0

    _cc3_set_cam_ddr_i2c_idle ();       // SDA=1 SCL=1
    ack = 0;

    //if(I2C_SDA)                     // sample SDA
    if (REG (GPIO_IOPIN) & 0x00800000) {
      ack |= 1;
      break;
    }

    _cc3_set_cam_ddr_i2c_write ();      // SDA=0 SCL=0

  }

  // Send Stop Bit 
  _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SCL);        // SDA=0 SCL=1
  _cc3_set_cam_ddr_i2c_idle (); // SDA=1 SCL=1

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
int cc3_set_raw_register (uint8_t address, uint8_t value)
{
  unsigned int data[3];
  int to;
  data[0] = _cc3_g_current_camera_state.camera_type;
  data[1] = address;
  data[2] = value;
  to = 0;
  while (_cc3_i2c_send (3, data)) {
    to++;
    if (to > 3)
      return 0;
  }
  _cc3_delay_us_4 (1);
  return 1;
}


/**
 * Sets the resolution, also updates cc3_g_current_frame width and height
 * Takes enum CC3_LOW_RES and CC3_HIGH_RES.
 * WARNING: Clears ROI, COI, sampling mode etc!
 */
int cc3_set_resolution (cc3_camera_resolution_t cam_res)
{
  _cc3_g_current_camera_state.resolution = cam_res;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
  cc3_frame_default ();

  return 1;
}

/**
 * This sets the hardware colorspace that comes out of the camera.
 * You can choose between CC3_RGB or CC3_YCRCB.  In RGB mode, then
 * address pixels with CC3_RED, CC3_GREEN, CC3_BLUE, and CC3_GREEN2
 * in YCrCb mode, use CC3_CR, CC3_Y, CC3_CB, CC3_Y2 when indexing
 * the pixel array.
 */
int cc3_set_colorspace (cc3_colorspace_t colorspace)
{
  _cc3_g_current_camera_state.colorspace = colorspace;
  _cc3_set_register_state ();
  return 1;
}


int cc3_set_framerate_divider (uint8_t rate_divider)
{
  _cc3_g_current_camera_state.clock_divider = rate_divider;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
  return 1;
}

int cc3_set_auto_exposure (bool exp)
{
  _cc3_g_current_camera_state.auto_exposure = exp;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
  return 1;
}

int cc3_set_auto_white_balance (bool awb)
{
  _cc3_g_current_camera_state.auto_white_balance = awb;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
  return 1;
}

int cc3_set_brightness (uint8_t level)
{
  _cc3_g_current_camera_state.brightness = level;
  _cc3_set_register_state ();   // XXX Don't reset all of them, this is just quick and dirty...
  return 1;
}

int cc3_set_contrast (uint8_t level)
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
