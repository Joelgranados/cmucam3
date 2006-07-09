/*
 * Copyright 2006  Anthony Rowe and Adam Goode
 *
 * This file is part of cc3.
 *
 * cc3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * cc3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cc3; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#ifndef CC3_PIN_DEFINES_H
#define CC3_PIN_DEFINES_H

#define CMUCAM_BOARD_VERSION_1
//#define CMUCAM_BOARD_VERSION_0_9
/*
 1 = output 0 = input

 Pin to Hex Helper
 3 3 2 2 | 2 2 2 2 | 2 2 2 2 | 1 1 1 1 | 1 1 1 1 | 1 1     |         |
 1 0 9 8 | 7 6 5 4 | 3 2 1 0 | 9 8 7 6 | 5 4 3 2 | 1 0 9 8 | 7 6 5 4 | 3 2 1 0
*/
#ifdef CMUCAM_BOARD_VERSION_1
/* PIN DEF for version 1.0  */

//#define _CC3_DEFAULT_PORT_DIR		0x003EFD99
#define _CC3_DEFAULT_PORT_DIR		0x003EBDC9
//#define DEFAULT_PORT_DIR	0x0 | BUF_WEE | CAM_RESET | BUF_WRST | BUF_RRST | BUF_RCK | BUF_RESET

// I2C Config Constants
#define _CC3_I2C_PORT_DDR_IDLE		0x001EBDC9
#define _CC3_I2C_PORT_DDR_READ_SDA	0x007EBDC9
#define _CC3_I2C_PORT_DDR_READ_SCL	0x00BEBDC9
#define _CC3_I2C_PORT_DDR_WRITE		0x00FEBDC9

// Camera Bus Constants
#define _CC3_CAM_VSYNC		0x10000
#define _CC3_BUF_WEE		0x400
//#define _CC3_CAM_RESET		0x80
#define _CC3_CAM_RESET	        0x8000	
#define _CC3_CAM_HREF		0x4
#define _CC3_CAM_SCL		0x400000
#define _CC3_CAM_SDA		0x800000

// Camera FIFO Constants
#define _CC3_BUF_WRST		0x2000
#define _CC3_BUF_RRST		0x1000
#define _CC3_BUF_RCK		0x800
#define _CC3_BUF_RESET		0x8

// IO pins
	

#define _CC3_SERVO_0 		0x00020000	
#define _CC3_SERVO_1 		0x00040000	
#define _CC3_SERVO_2 		0x00080000	
#define _CC3_SERVO_3 		0x00100000	

#define _CC3_LED_0	        0x20	
#define _CC3_LED_1	        _CC3_SERVO_2 
#define _CC3_LED_2	       	_CC3_SERVO_3 

#define _CC3_BUTTON             0x00004000

// SPI
#define _CC3_SPI_MASK           0x0000FF00
#define _CC3_SPI_PINSEL         0x00005500
#define _CC3_SPI_SPIF           0x00000080

// MMC
#define _CC3_MMC_CS             0x00004000

#endif

#ifdef CMUCAM_BOARD_VERSION_0_9

// PIN DEF for version 0.9+  
//#define _CC3_DEFAULT_PORT_DIR		0x003EFD99
#define _CC3_DEFAULT_PORT_DIR		0x003EBD89
//#define DEFAULT_PORT_DIR	0x0 | BUF_WEE | CAM_RESET | BUF_WRST | BUF_RRST | BUF_RCK | BUF_RESET

// I2C Config Constants
#define _CC3_I2C_PORT_DDR_IDLE		0x001EBD89
#define _CC3_I2C_PORT_DDR_READ_SDA	0x007EBD89
#define _CC3_I2C_PORT_DDR_READ_SCL	0x00BEBD89
#define _CC3_I2C_PORT_DDR_WRITE		0x00FEBD89

// Camera Bus Constants
#define _CC3_CAM_VSYNC		0x10000
#define _CC3_BUF_WEE		0x400
#define _CC3_CAM_RESET		0x80
#define _CC3_CAM_HREF		0x4
#define _CC3_CAM_SCL		0x400000
#define _CC3_CAM_SDA		0x800000

// Camera FIFO Constants
#define _CC3_BUF_WRST		0x2000
#define _CC3_BUF_RRST		0x1000
#define _CC3_BUF_RCK		0x800
#define _CC3_BUF_RESET		0x8

// IO pins
#define _CC3_LED_0	        0x8000	
#define _CC3_LED_1	        _CC3_LED_0
#define _CC3_LED_2	        _CC3_LED_0	


#define _CC3_SERVO_0 		0x00020000	
#define _CC3_SERVO_1 		0x00040000	
#define _CC3_SERVO_2 		0x00080000	
#define _CC3_SERVO_3 		0x00100000	

#define _CC3_BUTTON             0x00004000

// SPI
#define _CC3_SPI_MASK           0x0000FF00
#define _CC3_SPI_PINSEL         0x00005500
#define _CC3_SPI_SPIF           0x00000080

// MMC
#define _CC3_MMC_CS             0x00004000

#endif


#endif

