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


/* System Config */

/*
	PLL

   - Main clock F_OSC=14,7MHz @ Olimex LPC-P2129) [limits: 10 MHz to 25 MHz]
   - System should run at max. Frequency (60MHz) [limit: max 60 MHz]
   - Choose multiplier M=4
     so cclk = M * F_OSC= 4 * 14745000Hz = 58980000 Hz
   - MSEL-Bits in PLLCFG (bits 0-4) MSEL = M-1
   - F_CCO must be inbetween the limits 156 MHz to 320 MHz
     datasheet: F_CCO = F_OSC * M * 2 * P
   - choose devider P=2 => F_CCO = 14745000Hz * 4 * 2 * 2
     = 235920000 ~=236 MHz
   - PSEL0 (Bit5 in PLLCFG) = 1, PSEL1 (Bit6) = 0 (0b01)
*/
#define FOSC		14745000
#define PLL_M		4
#define MSEL		(PLL_M-1)
#define PSEL0 		5
#define PSEL1 		6

#define PLLE		0
#define PLLC		1

#define PLOCK		10

#define PLL_FEED1	0xAA
#define PLL_FEED2	0x55


/*
	MAM(Memory Accelerator Module)
	- choosen: MAM fully enabled = MAM-Mode 2
	- System-Clock cclk=59MHz -> 3 CCLKs are proposed as fetch timing
*/
#define MAM_MODE 	2
#define MAM_FETCH   3

/*
	VPB (V... Pheriphal Bus)
	- choosen: VPB should run at full speed -> devider VPBDIV=1
	=> pclk = cclk = 59MHz
*/
#define VPBDIV_VAL	1
// FIXME should be 1

/*
	SCB
*/
#define MEMMAP_BOOT_LOADER_MODE   0       // Interrupt vectors are re-mapped to Boot Block.
#define MEMMAP_USER_FLASH_MODE    (1<<0)  // Interrupt vectors are not re-mapped and reside in Flash.
#define MEMMAP_USER_RAM_MODE      (1<<1)  // Interrupt vectors are re-mapped to Static RAM.
