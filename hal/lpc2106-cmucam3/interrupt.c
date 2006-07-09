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


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "LPC2100.h"
#include "interrupt.h"
#include "cc3.h"
#include "cc3_pin_defines.h"
#include "serial.h"
#include "servo.h"

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
        disable_ext_interrupt ();
    }

//if(REG(VICRawIntr)&0x20)
    if (REG (TIMER1_IR) == 0x1) // Timer 1 MR0 interrupt fired
    {
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
