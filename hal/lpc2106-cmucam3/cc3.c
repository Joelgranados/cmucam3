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

void
image_send_direct (int size_x, int size_y)
{
  int x, y;
  cc3_pixbuf_load();
  putchar(1);
  putchar (size_x);
  putchar (size_y);
  for (y = 0; y < size_y; y++)
    {
      putchar (2);
      for (x = 0; x < size_x; x++)
	{
	  cc3_pixbuf_read();
	  putchar (cc3_g_current_pixel.channel[CC3_RED]);
	  putchar (cc3_g_current_pixel.channel[CC3_GREEN]);
	  putchar (cc3_g_current_pixel.channel[CC3_BLUE]);
	}
    }
  putchar (3);
  fflush(stdout);
}


void cc3_io_init(int BAUDRATE)
{

  //setvbuf(stdout, NULL, _IONBF, 0 );
  _cc3_uart0_setup(UART_BAUD(BAUDRATE), UART_8N1, UART_FIFO_8);

}


void cc3_pixbuf_load()
{
 unsigned int x, i;
  //REG(GPIO_IOCLR)=CAM_IE;  
  //while(frame_done!=1);
  printf( "1\r\n" );
  cc3_pixbuf_rewind(); 
  printf( "2\r\n" );
  _cc3_pixbuf_write_rewind();
  printf( "3\r\n" );
  while (!(REG (GPIO_IOPIN) & _CC3_CAM_VSYNC));	//while(CAM_VSYNC);
  while (REG (GPIO_IOPIN) & _CC3_CAM_VSYNC);	//while(!CAM_VSYNC);

  REG (GPIO_IOSET) = _CC3_BUF_WEE;
  printf( "4\r\n" );

  // wait for vsync to finish
  while (!(REG (GPIO_IOPIN) & _CC3_CAM_VSYNC));	//while(CAM_VSYNC);
  printf( "5\r\n" );

  enable_ext_interrupt ();

  printf( "6\r\n" );
  for (i = 0; i < 3; i++)
    {
      while (!(REG (GPIO_IOPIN) & _CC3_CAM_HREF));
      while (REG (GPIO_IOPIN) & _CC3_CAM_HREF);
    }

  printf( "7\r\n" );
//      delay();
  //REG(GPIO_IOCLR)=BUF_WEE;  //BUF_WEE=0return 1;
}


void cc3_pixbuf_skip(uint32_t size)
{
uint32_t i;
for(i=0; i<size; i++ )
    {
    _CC3_FIFO_READ_INC();
    _CC3_FIFO_READ_INC();
    _CC3_FIFO_READ_INC();
    _CC3_FIFO_READ_INC();
    }
}

/**
 * cc3_pixbuf_read():
 * loads cc3_g_current_pixel from fifo
 */
void cc3_pixbuf_read() 
{
  //if(hiresMode && !frame_dump) frame_skip_pixel();
  cc3_g_current_pixel.channel[0] = REG (GPIO_IOPIN);
  _CC3_FIFO_READ_INC();
  cc3_g_current_pixel.channel[1] = REG (GPIO_IOPIN);
   _CC3_FIFO_READ_INC ();
  cc3_g_current_pixel.channel[2] = REG (GPIO_IOPIN);
  _CC3_FIFO_READ_INC ();
  cc3_g_current_pixel.channel[3] = REG (GPIO_IOPIN);
  _CC3_FIFO_READ_INC ();

  cc3_g_current_pixel.channel[0] >>=24;
  cc3_g_current_pixel.channel[1] >>=24;
  cc3_g_current_pixel.channel[2] >>=24;
  cc3_g_current_pixel.channel[3] >>=24;


  //if(CAM_VSYNC && !buffer_mode )frame_write_reset();

}

/**
 * cc3_pixbuf_read3():
 * loads 3 bytes into cc3_g_current_pixel from fifo and skips second green. 
 */
void cc3_pixbuf_read3() 
{


}	
/**
 * cc3_pixbuf_rewind():
 * Rewinds the fifo 
 */
void cc3_pixbuf_rewind() 
{
  REG (GPIO_IOCLR) = _CC3_BUF_RRST;
  REG (GPIO_IOCLR) = _CC3_BUF_RCK;
  REG (GPIO_IOSET) = _CC3_BUF_RCK;
  REG (GPIO_IOCLR) = _CC3_BUF_RCK;
  REG (GPIO_IOSET) = _CC3_BUF_RRST;
}



void cc3_set_led(bool state)
{
if(state) REG (GPIO_IOSET) = _CC3_LED;
else REG( GPIO_IOCLR) = _CC3_LED;
}
/**
 * cc3_pixbuf_read_rows():
 * Using the cc3_frame_t reads rows taking into account virtual window and subsampling. 
 */
void cc3_pixbuf_read_rows( void* memory, uint32_t rows )
{


}
/**
 * cc3_pixbuf_set_roi():
 * Sets the region of interest in cc3_frame_t for virtual windowing. 
 */
int cc3_pixbuf_set_roi( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t x2)
{

}
/**
 * cc3_pixbuf_set_subsample():
 * Sets the subsampling step and mode in cc3_frame_t. 
 */
int cc3_pixbuf_set_subsample( cc3_subsample_mode_t mode, uint8_t x_step, uint8_t y_step )
{


}

/**
 * cc3_pixbuf_set_coi():
 * Sets the channel of interest 1 or all
 */
int cc3_pixbuf_set_coi( cc3_channel_t chan )
{

}



/**
 * cc3_camera_init():
 * 1) Enable Camera & FIFO Power
 * 2) Reset Camera
 * 3) call cc3_set functions for default state 
 *
 * Return:
 * 	1 sucessfully got acks back
 * 	0 failure (probably due to hardware?)
 */
int cc3_camera_init()
{
 REG (PCB_PINSEL0) =
    (REG (PCB_PINSEL0) & 0xFFFF0000) | UART0_PCB_PINSEL_CFG |
    UART1_PCB_PINSEL_CFG ; //| 0x50;
  REG (GPIO_IODIR) = _CC3_DEFAULT_PORT_DIR;
  //REG(GPIO_IOSET)=CAM_BUF_ENABLE;
  //REG (GPIO_IOCLR) = CAM_BUF_ENABLE;	// Change for AL440B
  REG (GPIO_IOCLR) = _CC3_BUF_RESET;
  _cc3_camera_reset();
  _cc3_fifo_reset ();

  cc3_g_camera_type = _CC3_OV6620;


}

/**
 * cc3_camera_kill():
 * Turn camera power off 
 * Turn fifo power off (may "cause picture to evaporate")
 */
void cc3_camera_kill()
{
// FUCK I need to be implemented

}


void _cc3_set_cam_ddr_i2c_idle()
{
  REG (GPIO_IODIR) = _CC3_I2C_PORT_DDR_IDLE;
  _cc3_delay_i2c ();
}

void _cc3_set_cam_ddr_i2c_write ()
{
  REG (GPIO_IODIR) = _CC3_I2C_PORT_DDR_WRITE;
  _cc3_delay_i2c ();
}


void _cc3_set_cam_ddr (volatile unsigned long val)
{
  //DDR(I2C_PORT,val);
  REG (GPIO_IODIR) = val;
  _cc3_delay_i2c ();
}



unsigned int _cc3_i2c_send (unsigned int num, unsigned int *buffer)
{
  unsigned int ack, i, k;
  unsigned int data;

  // Send Start Bit
  //I2C_SDA=0;  // needed because values can be reset by read-modify cycle
  REG (GPIO_IOCLR) = 0x00800000;
  _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SCL);	// SDA=0 SCL=1
  //I2C_SCL=0;  // needed because values can be reset by read-modify cycle
  REG (GPIO_IOCLR) = 0x00400000;
  _cc3_set_cam_ddr_i2c_write ();	// SDA=0 SCL=0

  // Send the Byte    
  for (k = 0; k != num; k++)
    {
      data = buffer[k];		// To avoid shifting array problems   
      for (i = 0; !(i & 8); i++)	// Write data    
	{
	  if (data & 0x80)
	    {
	      _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SDA);	// SDA=1 SCL=0
	      _cc3_set_cam_ddr_i2c_idle ();	// SDA=1 SCL=1
	    }
	  else
	    {
	      _cc3_set_cam_ddr_i2c_write ();	// SDA=0 SCL=0
	      _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SCL);	// SDA=0 SCL=1

	    }
	  while (!(REG (GPIO_IOPIN) & 0x00400000));
	  //while(!I2C_SCL);


	  if (data & 0x08)
	    {
	      _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SDA);	// SDA=1 SCL=0

	    }
	  else
	    {
	      _cc3_set_cam_ddr_i2c_write ();	// SDA=0 SCL=0
	    }

	  data <<= 1;
	}			// END OF 8 BIT FOR LOOP

      // Check ACK  <*************************************
      _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SDA);	// SDA=1 SCL=0

      _cc3_set_cam_ddr_i2c_idle ();	// SDA=1 SCL=1
      ack = 0;

      //if(I2C_SDA)                     // sample SDA
      if (REG (GPIO_IOPIN) & 0x00800000)
	{
	  ack |= 1;
	  break;
	}

      _cc3_set_cam_ddr_i2c_write ();	// SDA=0 SCL=0

    }

  // Send Stop Bit 
  _cc3_set_cam_ddr (_CC3_I2C_PORT_DDR_READ_SCL);	// SDA=0 SCL=1
  _cc3_set_cam_ddr_i2c_idle ();	// SDA=1 SCL=1

  return ack;

}

int cc3_set_raw_register( uint8_t address, uint8_t value)
{
 unsigned int data[3];
  int to;
  data[0] = cc3_g_camera_type;
  data[1] = address;
  data[2] = value;
  to = 0;
  while (_cc3_i2c_send (3, data))
    {
      to++;
      if (to > 3)
	return 0;
    }
  _cc3_delay_us_4 (1);
  return 1;
}



/**
 * cc3_set_resolution():
 * Sets the resolution, also updates cc3_g_current_frame width and height
 */
int cc3_set_resolution( cc3_camera_resolution_t cam_res) 
{
return 1;
}

int cc3_set_colorspace( cc3_colorspace_t colorspace )
{
return 1;
}

int cc3_set_framerate_divider( uint8_t rate_divider )
{
return 1;
}

int cc3_set_auto_exposure( bool exp )
{
return 1;
}

int cc3_set_auto_white_balance( bool awb )
{
return 1;
}

int cc3_set_brightness( uint8_t level)
{
return 1;
}

int cc3_set_contrast( uint8_t level)
{
return 1;
}

