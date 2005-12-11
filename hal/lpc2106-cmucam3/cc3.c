/**************************************************************************************
*  Initial CMUcam3 (cc3) data types and functions.
*
**************************************************************************************/
#include "cc3.h"
#include "cc3_pin_defines.h"
#include "cc3_hal.h"
#include <stdbool.h>
#include <stdio.h>
#include "serial.h"


// Globals used by CMUCam functions
cc3_pixel_t cc3_g_current_pixel;        // global that gets updated with pixbuf calls
cc3_frame_t cc3_g_current_frame;        // global that keeps clip, stride



void cc3_io_init (int BAUDRATE)
{
  uint8_t val;
  
  _cc3_uart0_setup (UART_BAUD (BAUDRATE), UART_8N1, UART_FIFO_8);

  val=setvbuf(stdout, NULL, _IONBF, 0 ); 
  /*
  if(val)
    {
      uart0_write("fail\r\n");
    }
  else
    {
      uart0_write("win\r\n");
    } 
  */
}


void cc3_pixbuf_load ()
{
    unsigned int x, i;
    //REG(GPIO_IOCLR)=CAM_IE;  
    //while(frame_done!=1);
    cc3_pixbuf_rewind ();
    _cc3_pixbuf_write_rewind ();
    while (!(REG (GPIO_IOPIN) & _CC3_CAM_VSYNC));       //while(CAM_VSYNC);
    while (REG (GPIO_IOPIN) & _CC3_CAM_VSYNC);  //while(!CAM_VSYNC);

    REG (GPIO_IOSET) = _CC3_BUF_WEE;

    // wait for vsync to finish
    while (!(REG (GPIO_IOPIN) & _CC3_CAM_VSYNC));       //while(CAM_VSYNC);

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


void _cc3_pixbuf_skip (uint32_t size)
{
    uint32_t i;
    for (i = 0; i < size; i++) {
        _CC3_FIFO_READ_INC ();
        _CC3_FIFO_READ_INC ();
        _CC3_FIFO_READ_INC ();
        _CC3_FIFO_READ_INC ();
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
    int8_t i;
    int16_t j;

    if (cc3_g_current_frame.y_loc < cc3_g_current_frame.y0) {
        // First read into frame
        // Skip top
        for (cc3_g_current_frame.y_loc = 0;
             cc3_g_current_frame.y_loc < cc3_g_current_frame.y0;
             cc3_g_current_frame.y_loc++)
            for (i = 0; i < cc3_g_current_frame.y_step; i++)
                _cc3_pixbuf_skip (cc3_g_current_frame.raw_width);
        cc3_g_current_frame.x_loc = 0;
        //printf( "Skipping Top %d %d\n",cc3_g_current_frame.x_loc, cc3_g_current_frame.y_loc );
    }

    if (cc3_g_current_frame.x_loc >= cc3_g_current_frame.x1) {
        // First read after line
        // Skip right and down 
        _cc3_pixbuf_skip (cc3_g_current_frame.raw_width -
                          (cc3_g_current_frame.x_loc *
                           cc3_g_current_frame.x_step));
        cc3_g_current_frame.x_loc = 0;
        cc3_g_current_frame.y_loc++;
        if (cc3_g_current_frame.y_step > 1) {
            // skip horizontally down the image
            for (j = 0; j < cc3_g_current_frame.y_step - 1; j++)
                _cc3_pixbuf_skip (cc3_g_current_frame.raw_width);
        }
        //printf( "Skipping right and down %d %d\n",cc3_g_current_frame.x_loc, cc3_g_current_frame.y_loc );
    }
    if (cc3_g_current_frame.x_loc < cc3_g_current_frame.x0) {
        // First read into line
        // Skip left 
        _cc3_pixbuf_skip (cc3_g_current_frame.x0 *
                          cc3_g_current_frame.x_step);
        cc3_g_current_frame.x_loc = cc3_g_current_frame.x0;
        //printf( "Skipping Left to %d %d\n", cc3_g_current_frame.x_loc, cc3_g_current_frame.y_loc );
    }
//printf( "[%d %d] \n",cc3_g_current_frame.x_loc, cc3_g_current_frame.y_loc );
    if (cc3_g_current_frame.y_loc > cc3_g_current_frame.y1)
        return 0;

// read the pixel
    for (i = 0; i < 4; i++) {
        if (cc3_g_current_frame.coi == i
            || cc3_g_current_frame.coi == CC3_ALL) {
            cc3_g_current_pixel.channel[i] = REG (GPIO_IOPIN);
            cc3_g_current_pixel.channel[i] >>= 24;
        }
        _CC3_FIFO_READ_INC ();
    }

    cc3_g_current_frame.x_loc++;
// skip horizontally for next read
    if (cc3_g_current_frame.x_step > 1) {
        if (cc3_g_current_frame.raw_width >
            (cc3_g_current_frame.x_loc + cc3_g_current_frame.x_step)) {
            // skip pixel
            _cc3_pixbuf_skip (cc3_g_current_frame.x_step - 1);
        }
    }

    return 1;
}

void _cc3_seek_top ()
{

}


void _cc3_seek_left ()
{

}

void _cc3_seek_right ()
{


}


void _cc3_pixbuf_read_0 ()
{
    cc3_g_current_pixel.channel[0] = REG (GPIO_IOPIN);
    _CC3_FIFO_READ_INC ();
    _CC3_FIFO_READ_INC ();
    _CC3_FIFO_READ_INC ();
    _CC3_FIFO_READ_INC ();
    cc3_g_current_pixel.channel[0] >>= 24;
}

void _cc3_pixbuf_read_1 ()
{
    _CC3_FIFO_READ_INC ();
    cc3_g_current_pixel.channel[1] = REG (GPIO_IOPIN);
    _CC3_FIFO_READ_INC ();
    _CC3_FIFO_READ_INC ();
    _CC3_FIFO_READ_INC ();
    cc3_g_current_pixel.channel[1] >>= 24;
}

void _cc3_pixbuf_read_2 ()
{
    _CC3_FIFO_READ_INC ();
    _CC3_FIFO_READ_INC ();
    cc3_g_current_pixel.channel[2] = REG (GPIO_IOPIN);
    _CC3_FIFO_READ_INC ();
    _CC3_FIFO_READ_INC ();
    cc3_g_current_pixel.channel[2] >>= 24;
}

/**
 * cc3_pixbuf_read():
 * loads cc3_g_current_pixel from fifo
 */
void _cc3_pixbuf_read_all ()
{
    //if(hiresMode && !frame_dump) frame_skip_pixel();
    cc3_g_current_pixel.channel[0] = REG (GPIO_IOPIN);
    _CC3_FIFO_READ_INC ();
    cc3_g_current_pixel.channel[1] = REG (GPIO_IOPIN);
    _CC3_FIFO_READ_INC ();
    cc3_g_current_pixel.channel[2] = REG (GPIO_IOPIN);
    _CC3_FIFO_READ_INC ();
    cc3_g_current_pixel.channel[3] = REG (GPIO_IOPIN);
    _CC3_FIFO_READ_INC ();

    //cc3_g_current_pixel.channel[0] >>= 24;
    //cc3_g_current_pixel.channel[1] >>= 24;
    //cc3_g_current_pixel.channel[2] >>= 24;
    //cc3_g_current_pixel.channel[3] >>= 24;
    //if(CAM_VSYNC && !buffer_mode )frame_write_reset();

}

/**
 * cc3_pixbuf_read3():
 * loads 3 bytes into cc3_g_current_pixel from fifo and skips second green. 
 * This is slightly faster if full image color information is not needed.
 */
void _cc3_pixbuf_read_all_3 ()
{
    cc3_g_current_pixel.channel[0] = REG (GPIO_IOPIN);
    _CC3_FIFO_READ_INC ();
    cc3_g_current_pixel.channel[1] = REG (GPIO_IOPIN);
    _CC3_FIFO_READ_INC ();
    _CC3_FIFO_READ_INC ();
    cc3_g_current_pixel.channel[3] = REG (GPIO_IOPIN);
    _CC3_FIFO_READ_INC ();

    cc3_g_current_pixel.channel[0] >>= 24;
    cc3_g_current_pixel.channel[1] >>= 24;
    cc3_g_current_pixel.channel[3] >>= 24;
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
}



void cc3_set_led (bool state)
{
    if (state)
        REG (GPIO_IOSET) = _CC3_LED;
    else
        REG (GPIO_IOCLR) = _CC3_LED;
}

/**
 * cc3_pixbuf_read_rows():
 * Using the cc3_frame_t reads rows taking into account the virtual window and subsampling. 
 * This should be the lowest level call that the user directly interacts with.
 */
int cc3_pixbuf_read_rows (void *memory, uint32_t rows)
{

    return 1;
}

/**
 * cc3_pixbuf_set_roi():
 * Sets the region of interest in cc3_frame_t for virtual windowing. 
 * This function changes the way data is read from the FIFO.
 * Returns 1 upon success and 0 on an out of bounds failure.
 */
int cc3_pixbuf_set_roi (uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
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
    if (x_step < 0 || y_step < 0)
        return 0;
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
    if (chan < 0 || chan > 4)
        return 0;               // Sanity check on bounds
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
    REG (PCB_PINSEL0) = (REG (PCB_PINSEL0) & 0xFFFF0000) | UART0_PCB_PINSEL_CFG | UART1_PCB_PINSEL_CFG; //| 0x50;
    REG (GPIO_IODIR) = _CC3_DEFAULT_PORT_DIR;
    //REG(GPIO_IOSET)=CAM_BUF_ENABLE;
    //REG (GPIO_IOCLR) = CAM_BUF_ENABLE;  // Change for AL440B
    REG (GPIO_IOCLR) = _CC3_BUF_RESET;
    _cc3_camera_reset ();
    _cc3_fifo_reset ();

    _cc3_g_current_camera_state.camera_type = _CC3_OV6620;      // XXX add autodetect code
    _cc3_g_current_camera_state.clock_divider = 0;
    _cc3_g_current_camera_state.brightness = -1;
    _cc3_g_current_camera_state.contrast = -1;
    _cc3_g_current_camera_state.auto_exposure = true;
    _cc3_g_current_camera_state.auto_white_balance = false;
    _cc3_g_current_camera_state.colorspace = CC3_RGB;
    _cc3_set_register_state ();

    cc3_frame_default ();
}

void cc3_frame_default ()
{
    cc3_g_current_frame.coi = CC3_ALL;
    cc3_g_current_frame.pixel_mode = CC3_BAYER;
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
// FUCK I need to be implemented

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
    _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SCL);      // SDA=0 SCL=1
    //I2C_SCL=0;  // needed because values can be reset by read-modify cycle
    REG (GPIO_IOCLR) = 0x00400000;
    _cc3_set_cam_ddr_i2c_write ();      // SDA=0 SCL=0

    // Send the Byte    
    for (k = 0; k != num; k++) {
        data = buffer[k];       // To avoid shifting array problems   
        for (i = 0; !(i & 8); i++)      // Write data    
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
        }                       // END OF 8 BIT FOR LOOP

        // Check ACK  <*************************************
        _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SDA);  // SDA=1 SCL=0

        _cc3_set_cam_ddr_i2c_idle ();   // SDA=1 SCL=1
        ack = 0;

        //if(I2C_SDA)                     // sample SDA
        if (REG (GPIO_IOPIN) & 0x00800000) {
            ack |= 1;
            break;
        }

        _cc3_set_cam_ddr_i2c_write ();  // SDA=0 SCL=0

    }

    // Send Stop Bit 
    _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SCL);      // SDA=0 SCL=1
    _cc3_set_cam_ddr_i2c_idle ();       // SDA=1 SCL=1

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
 */
int cc3_set_resolution (cc3_camera_resolution_t cam_res)
{
    _cc3_g_current_camera_state.resolution = cam_res;
    _cc3_set_register_state (); // XXX Don't reset all of them, this is just quick and dirty...
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
    _cc3_set_register_state (); // XXX Don't reset all of them, this is just quick and dirty...
    return 1;
}

int cc3_set_auto_exposure (bool exp)
{
    _cc3_g_current_camera_state.auto_exposure = exp;
    _cc3_set_register_state (); // XXX Don't reset all of them, this is just quick and dirty...
    return 1;
}

int cc3_set_auto_white_balance (bool awb)
{
    _cc3_g_current_camera_state.auto_white_balance = awb;
    _cc3_set_register_state (); // XXX Don't reset all of them, this is just quick and dirty...
    return 1;
}

int cc3_set_brightness (uint8_t level)
{
    _cc3_g_current_camera_state.brightness = level;
    _cc3_set_register_state (); // XXX Don't reset all of them, this is just quick and dirty...
    return 1;
}

int cc3_set_contrast (uint8_t level)
{
    _cc3_g_current_camera_state.contrast = level;
    _cc3_set_register_state (); // XXX Don't reset all of them, this is just quick and dirty...
    return 1;
}
