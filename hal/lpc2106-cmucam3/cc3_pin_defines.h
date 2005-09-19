#ifndef CC3_PIN_DEFINES_H
#define CC3_PIN_DEFINES_H


#define _CC3_OV6620 0xC0

/*
 1 = output 0 = input

 Pin to Hex Helper
 3 3 2 2 | 2 2 2 2 | 2 2 2 2 | 1 1 1 1 | 1 1 1 1 | 1 1     |         |
 1 0 9 8 | 7 6 5 4 | 3 2 1 0 | 9 8 7 6 | 5 4 3 2 | 1 0 9 8 | 7 6 5 4 | 3 2 1 0
*/

#define _CC3_DEFAULT_PORT_DIR		0x002EBD89
//#define DEFAULT_PORT_DIR	0x0 | BUF_WEE | CAM_RESET | BUF_WRST | BUF_RRST | BUF_RCK | BUF_RESET

// I2C Config Constants
#define _CC3_I2C_PORT_DDR_IDLE		0x000EBD89
#define _CC3_I2C_PORT_DDR_READ_SDA	0x006EBD89
#define _CC3_I2C_PORT_DDR_READ_SCL	0x00AEBD89
#define _CC3_I2C_PORT_DDR_WRITE		0x00EEBD89

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
#define _CC3_LED		0x8000

#define _CC3_SERVO_0 		0x00020000	
#define _CC3_SERVO_1 		0x00040000	
#define _CC3_SERVO_2 		0x00080000	
#define _CC3_SERVO_3 		0x00100000	




#endif

