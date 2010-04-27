/*
 * Copyright 2006-2010  Anthony Rowe and Adam Goode
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


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "LPC2100.h"
#include "interrupt.h"
#include "cc3.h"
#include "cc3_pin_defines.h"
#include "serial.h"
#include "servo.h"
#include "cc3_hal.h"

#define VIC_MSK_EINT0_DCLK	0x04000

volatile uint32_t row_done; 
volatile uint32_t dclk_cnt; 
volatile uint8_t row_buf[ROW_BUF_LEN];

__attribute__((section(".boot.fiq")))
void fast_interrupt (void)
{
  if(dclk_cnt<ROW_BUF_LEN ) {
	  row_buf[dclk_cnt]=(REG(GPIO_IOPIN)>>24); 
  	  dclk_cnt++;
  } else {
  row_done=1;	
  REG (VICIntEnClr) = VIC_MSK_EINT0_DCLK;
  }
 REG (SYSCON_EXTINT) = 0x1;  // clear EINT0
}
