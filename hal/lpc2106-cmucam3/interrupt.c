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

void disable_ext_interrupt (void)
{
    // Enable bit 14, which is the external interrupt 0...
    REG (VICIntEnClr) = 0x4000;
    REG (PCB_PINSEL1) = 0x0;    // Switch from EINT0 mode to GPIO

}


void enable_ext_interrupt (void)
{
    REG (SYSCON_EXTINT) = 0x1;
    // Enable bit 14, which is the external interrupt 0...
    REG (VICIntEnable) = 0x4000;
    // Clear the interrupt status flag...
    REG (PCB_PINSEL1) = 0x1;    // Switch from GPIO to EIN0 mode
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


void interrupt (void)
{
//asm volatile ( "msr cpsr_c,0x1F" ); // MODE_SYS

    //printf( "Got interrupt: %d\r\n",REG(VICRawIntr) );
    if (REG (VICRawIntr) & 0x4000) {
        // Triggered on VREF telling when a frame is complete.
        // Simply disable the FIFO once the frame has been captured. 
        REG (GPIO_IOCLR) = _CC3_BUF_WEE;        //BUF_WEE=0
	// You need to do this because if the read and write pointers on the FIFO
	// are near each other the FIFO gets angry
	_cc3_pixbuf_write_rewind ();
        disable_ext_interrupt ();
    }

//if(REG(VICRawIntr)&0x20)
    if (REG (TIMER1_IR) == 0x1) // Timer 1 MR0 interrupt fired
    {
   //uart0_write("Servo Int\r\n" );
        //printf( "Got timer 1 interrupt!\r\n" );
        _cc3_servo_int ();
        REG (TIMER1_IR) = 0x01; // clear pending flag 
    }

//asm volatile ( "msr cpsr_c,0xDF" );  // MODE_SYS | I_BIT | F_BIT
}


void undefined (void)
{
    uart0_write ("undefined instruction!\r\n");
    // XXX: tell us which instruction!
    exit (-1);
}

void swi (void)
{
    uart0_write ("swi!\r\n");
    // XXX: tell us something
    exit (-1);
}

void prefetch_abort (void)
{
    register volatile char *r_14 asm ("r14");
    long prev_pc = (long) r_14 - 4;
    uart0_write ("prefetch abort!\r\n");
    uart0_write_hex (prev_pc);
    // XXX: register dump
    exit (-1);
}

void data_abort (void)
{
    register volatile char *r_14 asm ("r14");
    long prev_pc = (long) r_14 - 8;
    uart0_write ("data abort!\r\n");
    uart0_write_hex (prev_pc);
    // XXX: register dump
    exit (-1);
}
