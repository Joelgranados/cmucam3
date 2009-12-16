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

volatile bool _cc3_button_trigger;

void disable_ext_interrupt (void)
{
    // Disable bit 14, which is the external interrupt 0...
    REG (VICIntEnClr) = 0x4000;
    // Switch from EINT0 mode to GPIO
    REG (PCB_PINSEL1) = 0x0;
    // Clear the interrupt
    REG (SYSCON_EXTINT) = 0x1;
}


void enable_ext_interrupt (void)
{
    // Switch from GPIO to EINT0 mode
    REG (PCB_PINSEL1) = 0x1;
    // Enable bit 14, which is the external interrupt 0...
    REG (VICIntEnable) = 0x4000;
}

void enable_servo_interrupt (void)
{
    //uart0_write("Enable Servo Int\r\n" );
    REG (VICIntEnable) = 0x20;
}

void disable_servo_interrupt (void)
{
    REG (VICIntEnClr) = 0x20;
}

void enable_button_interrupt (void)
{
  //uart0_write("button int enable\r\n");
  // pin select
  REG(PCB_PINSEL0) = (REG(PCB_PINSEL0) &
                      ~_CC3_BUTTON_PINSEL_MASK) | _CC3_BUTTON_PINSEL;

  // vic bit 15: EINT1
  REG (VICIntEnable) = 0x8000;
}

void disable_button_interrupt (void)
{
  //uart0_write("button int disable\r\n");

  // vic bit 15: EINT1
  REG (VICIntEnClr) = 0x8000;

  // pin select back to GPIO
  REG(PCB_PINSEL0) = (REG(PCB_PINSEL0) & ~_CC3_BUTTON_PINSEL_MASK);

  // clear the interrupt
  REG (SYSCON_EXTINT) = 0x2;
}

void interrupt (void)
{
    if (REG (VICRawIntr) & 0x8000) {
      // button press
      //uart0_write("button int\r\n");
      _cc3_button_trigger = true;
      disable_button_interrupt ();
    }
}

void swi (void)
{
    uart0_write ("swi!\r\n");
    // XXX: tell us something
    exit (-1);
}


static void panic_blink(int delay) {
  // set LEDs to output
  REG (GPIO_IODIR) |= _CC3_LED_0;

  // blink
  while (true) {
    REG (GPIO_IOCLR) = _CC3_LED_0;
    cc3_timer_wait_ms(500);
    REG (GPIO_IOSET) = _CC3_LED_0;
    cc3_timer_wait_ms(delay);
  }
}

void undefined (void)
{
  panic_blink(500);
}

void prefetch_abort (void)
{
    register volatile char *r_14 asm ("r14");
    long prev_pc = (long) r_14 - 4;
    uart0_write ("prefetch abort!\r\n");
    uart0_write_hex (prev_pc);
    // XXX: register dump

    panic_blink(1000);
}

void data_abort (void)
{
    register volatile char *r_14 asm ("r14");
    long prev_pc = (long) r_14 - 8;
    uart0_write ("data abort!\r\n");
    uart0_write_hex (prev_pc);
    // XXX: register dump

    panic_blink(1500);
}
