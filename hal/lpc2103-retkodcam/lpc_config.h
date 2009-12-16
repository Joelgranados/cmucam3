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


/* System Config */

/*
	PLL

   - Main clock F_OSC=20MHz
   - System should run at max. Frequency (70MHz)
   - Choose multiplier M=3 = 60 MHz
   - MSEL-Bits in PLLCFG (bits 0-4) MSEL = M-1
   - F_CCO must be inbetween the limits 156 MHz to 320 MHz
     datasheet: F_CCO = F_OSC * M * 2 * P
   - choose divider P=2 => F_CCO = 20000000Hz * 3 * 2 * 2
     = 240000000 = 240 MHz
   - PSEL0 (Bit5 in PLLCFG) = 1, PSEL1 (Bit6) = 0 (0b01)
*/
#define FOSC		20000000
#define PLL_M		3
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
	- System-Clock cclk=60MHz -> 3 CCLKs are proposed as fetch timing
*/
#define MAM_MODE 	2
#define MAM_FETCH   3

/*
	APB (A... Pheriphal Bus)
	- choosen: APB should run at full speed -> divider APBDIV=1
	=> pclk = cclk = 59MHz
*/
#define APBDIV_VAL	1
// FIXME should be 1

/*
	SCB
*/
#define MEMMAP_BOOT_LOADER_MODE   0       // Interrupt vectors are re-mapped to Boot Block.
#define MEMMAP_USER_FLASH_MODE    (1<<0)  // Interrupt vectors are not re-mapped and reside in Flash.
#define MEMMAP_USER_RAM_MODE      (1<<1)  // Interrupt vectors are re-mapped to Static RAM.
