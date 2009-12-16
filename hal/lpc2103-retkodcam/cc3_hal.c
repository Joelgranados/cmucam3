/*
 * Copyright 2006-2007  Anthony Rowe and Adam Goode
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


#include "cc3_hal.h"
#include "cc3_pin_defines.h"
#include "interrupt.h"
#include "serial.h"
#include <stdio.h>


// called from startup.s before main!
void
_cc3_system_setup (void)
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

  // --- set APB speed ---
  REG(SYSCON_APBDIV) = APBDIV_VAL;

  // --- map INT-vector ---
  REG(SYSCON_MEMMAP) = MEMMAP_USER_FLASH_MODE;

#ifdef FASTIO
  // enable FASTIO
  REG ( SYSCON_SCS ) = 0x1;
#endif

  REG (PCB_PINSEL0) = (REG (PCB_PINSEL0) & 0xFFFF0000) | UART0_PCB_PINSEL_CFG | UART1_PCB_PINSEL_CFG;  //| 0x50;
  //REG (PCB_PINSEL0) = (REG (PCB_PINSEL0) & 0xFFFF0000) | UART0_PCB_PINSEL_CFG | UART1_PCB_PINSEL_CFG; //| 0x50;
  REG (GPIO_IODIR) = _CC3_DEFAULT_PORT_DIR;
  REG (GPIO_IOSET) = 0x40000;  // extinguish LED

  //REG(PCB_PINSEL1) = 0x1;  // External interrupt 0

  enable_button_interrupt ();

  // Setup timer0 to count by milliseconds starting from 0
  REG(TIMER0_TCR)=0;   // turn off timer
  REG(TIMER0_MCR)=0;    // disable interrupt
  REG(TIMER0_TC)=0;    // clear counter
  REG(TIMER0_PC)=0;    // clear prescale count
  REG(TIMER0_PR) = (int)((FOSC*PLL_M)/1000);  // every 1 ms
  REG(TIMER0_TCR)=1;   // start the timer
}


/*****************************************
*                                        *
* CAMERA REGISTER CONTROL FUNCTIONS      *
*                                        *
******************************************/
/**
 * This is a private function that throbs the camera
 * reset pin.
 */
void _cc3_camera_reset ()
{
  // Reset the Camera
  REG (GPIO_IOCLR) = _CC3_CAM_RESET;
  cc3_timer_wait_ms(1);
  REG (GPIO_IOSET) = _CC3_CAM_RESET;
  cc3_timer_wait_ms(1);
}
