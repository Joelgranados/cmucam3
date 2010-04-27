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


#ifndef CC3_PIN_DEFINES_H
#define CC3_PIN_DEFINES_H


/*
 1 = output 0 = input

 Pin to Hex Helper
 3 3 2 2 | 2 2 2 2 | 2 2 2 2 | 1 1 1 1 | 1 1 1 1 | 1 1     |         |
 1 0 9 8 | 7 6 5 4 | 3 2 1 0 | 9 8 7 6 | 5 4 3 2 | 1 0 9 8 | 7 6 5 4 | 3 2 1 0
*/


#define _CC3_DEFAULT_PORT_DIR		0x430B0

// Camera Bus Constants
#define _CC3_CAM_VBLK			0x8000
#define _CC3_CAM_VBLK_PINSEL_MASK	0xc0000000		
#define _CC3_CAM_VBLK_PINSEL		0x40000000

#define _CC3_CAM_DCLK			0x10000
#define _CC3_CAM_DCLK_PINSEL_MASK	0x00000003		
#define _CC3_CAM_DCLK_PINSEL		0x00000001

#define _CC3_CAM_HBLK			0x20000
#define _CC3_CAM_HBLK_PINSEL_MASK	0x0000000c		
#define _CC3_CAM_HBLK_PINSEL		0x00000008

#define _CC3_CAM_ENABLE		0x1000
#define _CC3_CAM_RESET		0x2000

// IO pins
#define _CC3_LED_0	        0x40000

// button
#define _CC3_BUTTON             0x00004000
#define _CC3_BUTTON_PINSEL_MASK 0x30000000
#define _CC3_BUTTON_PINSEL      0x10000000

// SPI
#define _CC3_SPI_MASK           0x0000FF00
#define _CC3_SPI_PINSEL         0x00005500
#define _CC3_SPI_PINSEL_DISABLE 0x00000000
#define _CC3_SPI_SPIF           0x00000080

// I2C
#define _CC3_I2C0_MASK          0x000000F0
#define _CC3_I2C0_PINSEL        0x00000050

// MMC
#define _CC3_MMC_CS             0x00004000


#endif

