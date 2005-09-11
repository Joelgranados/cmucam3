#ifndef CC3_H
#define CC3_H

#include "LPC2100.h"


#define OV6620 0xC0
#define OV7620 0x42


// Move to the next byte in the FIFO 
#define FIFO_READ_INC() REG(GPIO_IOSET)=BUF_RCK; REG(GPIO_IOCLR)=BUF_RCK


int _cc3_camera_set_reg (int reg, int val);	// Set a camera register value on the Omnivision camera
void camera_setup ();		// Set default values for the camera
void camera_reset ();		// Toggle Reset Pin for the camera

void fifo_reset ();		// FIFO hardware reset
void fifo_load_frame ();	// Load a single Image from the Camera into the FIFO 
void fifo_write_reset ();	// Reset the FIFO write pointer
void fifo_read_reset ();	// Reset the FIFO read pointer
void fifo_read_pixel ();	// Read a pixel from the FIFO in the xxx_pixel globals
void fifo_skip_pixel ();	// Fast Forward over a pixel (4 bytes) in the FIFO

void image_send_uart (unsigned char *img, int size_x, int size_y);	// Send memory out UART 
									// using CMUcam format

void image_fifo_to_mem (unsigned char *img, int size_x, int size_y);	// Copy lines from FIFO
									// to memory.  Note, must
									// copy entire lines at a time.

void image_send_direct (int size_x, int size_y);	// Send an image directly from the FIFO without
							// buffering it on the processor first

void InitialiseI2C (void);
void delay (void);
void delay_i2c ();
void delay_us_4 (int cnt);
void set_cam_ddr_i2c_idle ();
void set_cam_ddr_i2c_write ();
void set_cam_ddr (volatile unsigned long val);
unsigned int i2c_send (unsigned int num, unsigned int *buffer);

// 1 = output 0 = input
#define DEFAULT_PORT_DIR	0x002EBD89
//#define DEFAULT_PORT_DIR	0x0 | BUF_WEE | CAM_RESET | BUF_WRST | BUF_RRST | BUF_RCK | BUF_RESET

// I2C Config Constants
#define I2C_PORT_DDR_IDLE	0x000EBD89
#define I2C_PORT_DDR_READ_SDA	0x006EBD89
#define I2C_PORT_DDR_READ_SCL	0x00AEBD89
#define I2C_PORT_DDR_WRITE	0x00EEBD89


// Camera Bus Constants
//#define CAM_BUF_ENABLE 	0x8
#define CAM_VSYNC	0x10000
#define BUF_WEE		0x400
#define CAM_RESET	0x80
#define CAM_HREF	0x4
#define CAM_SCL		0x400000
#define CAM_SDA		0x800000

// Camera FIFO Constants
#define BUF_WRST	0x2000
#define BUF_RRST	0x1000
#define BUF_RCK		0x800
#define BUF_RESET	0x8

#define LED		0x8000


#endif
