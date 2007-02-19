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
#include "serial.h"
#include <stdio.h>


_cc3_camera_state_t _cc3_g_current_camera_state;


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

  // --- set VPB speed ---
  REG(SYSCON_VPBDIV) = VPBDIV_VAL;

  // --- map INT-vector ---
  REG(SYSCON_MEMMAP) = MEMMAP_USER_FLASH_MODE;

  //REG(PCB_PINSEL1) = 0x1;  // External interrupt 0

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
  _cc3_delay_us_4 (1);
  REG (GPIO_IOSET) = _CC3_CAM_RESET;
  _cc3_delay_us_4 (1);
  REG (GPIO_IOCLR) = _CC3_CAM_RESET;
  _cc3_delay_us_4 (1);
}


/**
 * This function resets the fifo chip.
 */
void _cc3_fifo_reset ()
{
  REG (GPIO_IOCLR) = _CC3_BUF_RESET;
  _cc3_delay_us_4 (1);
  REG (GPIO_IOSET) = _CC3_BUF_RESET;

  REG (GPIO_IOCLR) = _CC3_BUF_WEE;
  REG (GPIO_IOCLR) = _CC3_BUF_WRST;
  REG (GPIO_IOCLR) = _CC3_BUF_RRST;
  REG (GPIO_IOCLR) = _CC3_BUF_RCK;
  REG (GPIO_IOSET) = _CC3_BUF_RCK;
  REG (GPIO_IOCLR) = _CC3_BUF_RCK;
  _cc3_delay_us_4 (1);
  REG (GPIO_IOSET) = _CC3_BUF_WRST;
  REG (GPIO_IOSET) = _CC3_BUF_RRST;

}

/**
 * _cc3_delay_us_4()
 *
 * This function delays in intervauls of 4us
 * without using the timer.
 */
void _cc3_delay_us_4 (int cnt)
{
  volatile int i, x;
  for (i = 0; i < cnt; i++)
    for (x = 0; x < 10; x++);
}

void _cc3_delay_sccb ()
{
  volatile int x;
  //for (x = 0; x < 1000; x++);
  for (x = 0; x < 25; x++);

}

void _cc3_pixbuf_write_rewind ()
{
  REG (GPIO_IOCLR) = _CC3_BUF_WEE;
  REG (GPIO_IOCLR) = _CC3_BUF_WRST;
  _cc3_delay_us_4 (1);
  REG (GPIO_IOSET) = _CC3_BUF_WRST;
}



/*
 * This function goes through the register state structures and sets the corresponding camera registers.
 * It also updates any internal camera structures such as resolution that may change.
 */
bool _cc3_set_register_state ()
{
  bool result = true;

  switch (_cc3_g_current_camera_state.camera_type) {
  case _CC3_OV6620:
    // Set the right data bus mode
    result &= cc3_camera_set_raw_register (0x14, 0x20);

    // set the power state
    if (_cc3_g_current_camera_state.power_state) {
      // wake up the camera
      result &= cc3_camera_set_raw_register (0x3F, 0x02);
    } else {
      // sleep the camera
      result &= cc3_camera_set_raw_register (0x3F, 0x12);
    }

    // Set the resolution and update the size flags
    if (_cc3_g_current_camera_state.resolution == CC3_CAMERA_RESOLUTION_LOW) {
      _cc3_g_current_camera_state.raw_width = CC3_LO_RES_WIDTH; // 88 * 2;
      _cc3_g_current_camera_state.raw_height = CC3_LO_RES_HEIGHT;       // 144;
      result &= cc3_camera_set_raw_register (0x14, 0x20);
    }
    else {
      _cc3_g_current_camera_state.raw_width = CC3_HI_RES_WIDTH; // 176 * 2;
      _cc3_g_current_camera_state.raw_height = CC3_HI_RES_HEIGHT;       //288;
      result &= cc3_camera_set_raw_register (0x14, 0x00);
    }

    if (_cc3_g_current_camera_state.auto_exposure) {
      result &= cc3_camera_set_raw_register (0x13, 0x21);
    }
    else {
      // No auto gain, so lets set brightness and contrast if need be
      result &= cc3_camera_set_raw_register (0x13, 0x20);
      if (_cc3_g_current_camera_state.brightness != -1)
        result &= cc3_camera_set_raw_register (0x06,
                                               (_cc3_g_current_camera_state.
                                                brightness & 0xFF));

      if (_cc3_g_current_camera_state.contrast != -1)
        result &= cc3_camera_set_raw_register (0x05,
                                               (_cc3_g_current_camera_state.
                                                contrast & 0xFF));
    }
    // Set Colorspace and Auto White Balance
    result &= cc3_camera_set_raw_register (0x12,
                                           0x20 |
                                           (_cc3_g_current_camera_state.
                                            auto_white_balance << 2)
                                           | (_cc3_g_current_camera_state.
                                              colorspace << 3));
    // Set Frame Clock rate divider
    result &=
      cc3_camera_set_raw_register (0x11,
                                   _cc3_g_current_camera_state.clock_divider);

    break;

  case _CC3_OV7620:
    // XXX I need code.  The CMUcam2 is kind of wrong, so lets fix it...
    result = false;
    break;
  }

  return result;
}
